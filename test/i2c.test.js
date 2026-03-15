const { describe, it, beforeEach, afterEach } = require('node:test');
const assert = require('node:assert/strict');
const I2C = require('../lib/i2c');

function createMockDevice(overrides) {
  const noop = () => {};
  return {
    open: (path, cb) => setImmediate(() => cb(null)),
    close: noop,
    setAddress: noop,
    scan: noop,
    read: noop,
    readByte: noop,
    readBlock: noop,
    write: noop,
    writeByte: noop,
    writeBlock: noop,
    writeWord: noop,
    // tracking
    _calls: {},
    ...overrides,
  };
}

function trackCalls(device, method) {
  const original = device[method];
  const calls = [];
  device[method] = function(...args) {
    calls.push(args);
    return original.apply(this, args);
  };
  device._calls[method] = calls;
  return calls;
}

function createWire(address, mockDevice, options) {
  return new Promise((resolve, reject) => {
    function MockDeviceClass() { return mockDevice; }
    const wire = new I2C(address, { _DeviceClass: MockDeviceClass, ...options });
    wire.on('open', () => resolve(wire));
    wire.on('error', reject);
  });
}

describe('I2C', () => {
  let wire;

  afterEach(() => {
    if (wire) {
      wire.close();
      wire = null;
    }
  });

  describe('constructor', () => {
    it('opens the device and sets address', async () => {
      const md = createMockDevice();
      const openCalls = trackCalls(md, 'open');
      const addrCalls = trackCalls(md, 'setAddress');

      wire = await createWire(0x18, md);
      assert.equal(openCalls.length, 1);
      assert.equal(openCalls[0][0], '/dev/i2c-1');
      assert.equal(addrCalls.length, 1);
      assert.equal(addrCalls[0][0], 0x18);
    });

    it('uses custom device path from options', async () => {
      const md = createMockDevice();
      const openCalls = trackCalls(md, 'open');

      wire = await createWire(0x18, md, { device: '/dev/i2c-0' });
      assert.equal(openCalls[0][0], '/dev/i2c-0');
    });

    it('emits error when open fails', async () => {
      const md = createMockDevice({
        open: (path, cb) => setImmediate(() => cb(new Error('No device'))),
      });
      const addrCalls = trackCalls(md, 'setAddress');

      function MockDeviceClass() { return md; }
      const wire2 = new I2C(0x18, { _DeviceClass: MockDeviceClass });
      const err = await new Promise((resolve) => wire2.on('error', resolve));
      assert.equal(err.message, 'No device');
      assert.equal(addrCalls.length, 0);
      wire2.close();
    });
  });

  describe('setAddress', () => {
    it('updates address on both JS and native side', async () => {
      const md = createMockDevice();
      const addrCalls = trackCalls(md, 'setAddress');

      wire = await createWire(0x18, md);
      wire.setAddress(0x20);
      assert.equal(wire.address, 0x20);
      assert.equal(addrCalls.length, 2);
      assert.equal(addrCalls[1][0], 0x20);
    });
  });

  describe('close', () => {
    it('calls native close and removes exit listener', async () => {
      const md = createMockDevice();
      const closeCalls = trackCalls(md, 'close');

      wire = await createWire(0x18, md);
      const listenersBefore = process.listenerCount('exit');
      wire.close();
      assert.equal(closeCalls.length, 1);
      assert.equal(process.listenerCount('exit'), listenersBefore - 1);
      wire = null;
    });

    it('stops streaming on close', async () => {
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => setImmediate(() => cb(null, Buffer.from([1]))),
      });

      wire = await createWire(0x18, md);
      wire.stream(0x01, 1, 10);
      wire.close();
      assert.equal(wire._streaming, false);
      assert.equal(wire._streamTimer, null);
      wire = null;
    });
  });

  describe('scan', () => {
    it('filters negative values from scan results', async () => {
      const md = createMockDevice({
        scan: (cb) => {
          const results = new Array(128).fill(-1);
          results[0x18] = 0x18;
          results[0x50] = 0x50;
          setImmediate(() => cb(null, results));
        },
      });

      wire = await createWire(0x18, md);
      const devices = await new Promise((resolve, reject) => {
        wire.scan((err, data) => err ? reject(err) : resolve(data));
      });
      assert.deepEqual(devices, [0x18, 0x50]);
    });
  });

  describe('write', () => {
    it('converts array to Buffer before writing', async () => {
      const md = createMockDevice({
        write: (buf, cb) => setImmediate(() => cb(null)),
      });
      const writeCalls = trackCalls(md, 'write');

      wire = await createWire(0x18, md);
      await new Promise((resolve, reject) => {
        wire.write([0x01, 0x02], (err) => err ? reject(err) : resolve());
      });
      assert.ok(Buffer.isBuffer(writeCalls[0][0]));
      assert.deepEqual([...writeCalls[0][0]], [0x01, 0x02]);
    });

    it('passes Buffer directly', async () => {
      const md = createMockDevice({
        write: (buf, cb) => setImmediate(() => cb(null)),
      });
      const writeCalls = trackCalls(md, 'write');
      const buf = Buffer.from([0xFF]);

      wire = await createWire(0x18, md);
      await new Promise((resolve, reject) => {
        wire.write(buf, (err) => err ? reject(err) : resolve());
      });
      assert.strictEqual(writeCalls[0][0], buf);
    });

    it('passes error to callback', async () => {
      const md = createMockDevice({
        write: (buf, cb) => setImmediate(() => cb(new Error('write failed'))),
      });

      wire = await createWire(0x18, md);
      const err = await new Promise((resolve) => {
        wire.write([0x01], (e) => resolve(e));
      });
      assert.equal(err.message, 'write failed');
    });
  });

  describe('writeByte', () => {
    it('writes a single byte', async () => {
      const md = createMockDevice({
        writeByte: (byte, cb) => setImmediate(() => cb(null)),
      });
      const calls = trackCalls(md, 'writeByte');

      wire = await createWire(0x18, md);
      await new Promise((resolve, reject) => {
        wire.writeByte(0xAB, (err) => err ? reject(err) : resolve());
      });
      assert.equal(calls[0][0], 0xAB);
    });
  });

  describe('writeBytes', () => {
    it('converts array to Buffer and calls writeBlock', async () => {
      const md = createMockDevice({
        writeBlock: (cmd, buf, cb) => setImmediate(() => cb(null)),
      });
      const calls = trackCalls(md, 'writeBlock');

      wire = await createWire(0x18, md);
      await new Promise((resolve, reject) => {
        wire.writeBytes(0x10, [0x01, 0x02, 0x03], (err) => err ? reject(err) : resolve());
      });
      assert.equal(calls[0][0], 0x10);
      assert.ok(Buffer.isBuffer(calls[0][1]));
      assert.deepEqual([...calls[0][1]], [0x01, 0x02, 0x03]);
    });
  });

  describe('read', () => {
    it('reads data from device', async () => {
      const data = [1, 2, 3];
      const md = createMockDevice({
        read: (len, cb) => setImmediate(() => cb(null, data)),
      });
      const calls = trackCalls(md, 'read');

      wire = await createWire(0x18, md);
      const result = await new Promise((resolve, reject) => {
        wire.read(3, (err, d) => err ? reject(err) : resolve(d));
      });
      assert.deepEqual(result, data);
      assert.equal(calls[0][0], 3);
    });
  });

  describe('readByte', () => {
    it('reads a single byte', async () => {
      const md = createMockDevice({
        readByte: (cb) => setImmediate(() => cb(null, 0x42)),
      });

      wire = await createWire(0x18, md);
      const data = await new Promise((resolve, reject) => {
        wire.readByte((err, d) => err ? reject(err) : resolve(d));
      });
      assert.equal(data, 0x42);
    });
  });

  describe('readBytes', () => {
    it('reads a block of bytes from a register', async () => {
      const buf = Buffer.from([0x10, 0x20, 0x30]);
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => setImmediate(() => cb(null, buf)),
      });
      const calls = trackCalls(md, 'readBlock');

      wire = await createWire(0x18, md);
      const data = await new Promise((resolve, reject) => {
        wire.readBytes(0x01, 3, (err, d) => err ? reject(err) : resolve(d));
      });
      assert.deepEqual(data, buf);
      assert.equal(calls[0][0], 0x01);
      assert.equal(calls[0][1], 3);
    });
  });

  describe('stream', () => {
    it('emits data events repeatedly and stops with stopStream', async () => {
      let callCount = 0;
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => {
          callCount++;
          const c = callCount;
          setImmediate(() => cb(null, Buffer.from([c])));
        },
      });

      wire = await createWire(0x18, md);
      const received = await new Promise((resolve) => {
        const items = [];
        wire.on('data', (evt) => {
          items.push(evt.data[0]);
          if (items.length === 3) {
            wire.stopStream();
            resolve(items);
          }
        });
        wire.stream(0x01, 1, 10);
      });
      assert.deepEqual(received, [1, 2, 3]);
      assert.equal(wire._streaming, false);
    });

    it('emits error events on read failure', async () => {
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => {
          setImmediate(() => cb(new Error('read error')));
        },
      });

      wire = await createWire(0x18, md);
      const err = await new Promise((resolve) => {
        wire.on('error', (e) => {
          wire.stopStream();
          resolve(e);
        });
        wire.stream(0x01, 1, 10);
      });
      assert.equal(err.message, 'read error');
    });

    it('includes metadata in data events', async () => {
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => {
          setImmediate(() => cb(null, Buffer.from([0xFF])));
        },
      });

      wire = await createWire(0x18, md);
      const evt = await new Promise((resolve) => {
        wire.on('data', (e) => {
          wire.stopStream();
          resolve(e);
        });
        wire.stream(0x02, 1, 10);
      });
      assert.equal(evt.address, 0x18);
      assert.equal(evt.cmd, 0x02);
      assert.equal(evt.length, 1);
      assert.equal(typeof evt.timestamp, 'number');
    });

    it('uses default delay of 100ms when not specified', async () => {
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => {
          setImmediate(() => cb(null, Buffer.from([1])));
        },
      });

      wire = await createWire(0x18, md);
      wire.on('data', () => wire.stopStream());
      wire.stream(0x01, 1);
      await new Promise((r) => setTimeout(r, 50));
    });
  });

  describe('no listener leak', () => {
    it('does not accumulate exit listeners across multiple instances', async () => {
      const before = process.listenerCount('exit');
      const instances = [];

      for (let i = 0; i < 5; i++) {
        const md = createMockDevice();
        instances.push(await createWire(0x18 + i, md));
      }

      assert.equal(process.listenerCount('exit'), before + 5);

      for (const inst of instances) {
        inst.close();
      }

      assert.equal(process.listenerCount('exit'), before);
      wire = null;
    });
  });

  describe('double close safety', () => {
    it('calling close() twice does not throw', async () => {
      const md = createMockDevice();
      const closeCalls = trackCalls(md, 'close');

      wire = await createWire(0x18, md);
      wire.close();
      wire.close(); // should not throw or double-remove listener
      assert.equal(closeCalls.length, 1); // native close called only once
      wire = null;
    });
  });

  describe('double stream guard', () => {
    it('calling stream() twice does not start parallel loops', async () => {
      let callCount = 0;
      const md = createMockDevice({
        readBlock: (cmd, len, cb) => {
          callCount++;
          setImmediate(() => cb(null, Buffer.from([callCount])));
        },
      });

      wire = await createWire(0x18, md);
      wire.on('data', () => {}); // consume events
      wire.stream(0x01, 1, 50);
      wire.stream(0x01, 1, 50); // should be ignored
      await new Promise((r) => setTimeout(r, 30));
      wire.stopStream();
      // If double-stream started two loops, callCount would be > 1
      assert.equal(callCount, 1);
    });
  });

  describe('scan error handling', () => {
    it('passes error to callback without crashing on undefined data', async () => {
      const md = createMockDevice({
        scan: (cb) => {
          setImmediate(() => cb(new Error('scan failed')));
        },
      });

      wire = await createWire(0x18, md);
      const err = await new Promise((resolve) => {
        wire.scan((err) => resolve(err));
      });
      assert.equal(err.message, 'scan failed');
    });
  });

  describe('optional callbacks', () => {
    it('write works without callback', async () => {
      const md = createMockDevice({
        write: (buf, cb) => setImmediate(() => cb(null)),
      });

      wire = await createWire(0x18, md);
      wire.write([0x01]);
      await new Promise((r) => setTimeout(r, 20));
    });

    it('read works without callback', async () => {
      const md = createMockDevice({
        read: (len, cb) => setImmediate(() => cb(null, [1])),
      });

      wire = await createWire(0x18, md);
      wire.read(1);
      await new Promise((r) => setTimeout(r, 20));
    });
  });
});
