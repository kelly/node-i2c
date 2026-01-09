const rewire = require('rewire');
const assert = require('assert');

const i2c = rewire('../lib/i2c.js');

const mockWire = {
  scan: (callback) => callback(null, [0x27, 0x42]),
  open: (device, callback) => callback(null),
  setAddress: (address) => {},
  write: (buffer, callback) => callback(null),
  writeByte: (byte, callback) => callback(null),
  writeBlock: (cmd, buffer, callback) => callback(null),
  read: (len, callback) => callback(null, Buffer.from('hello')),
  readByte: (callback) => callback(null, 0x42),
  readBlock: (cmd, len, buffer, callback) => callback(null, Buffer.from('hello')),
  close: () => {}
};

i2c.__set__('wire', mockWire);

describe('i2c', () => {
  let wire;

  beforeEach(() => {
    wire = new i2c(0x27, { device: '/dev/i2c-1' });
  });

  it('should scan for devices', async () => {
    const devices = await wire.scanAsync();
    assert.deepStrictEqual(devices, [0x27, 0x42]);
  });

  it('should open a device', async () => {
    await wire.openAsync('/dev/i2c-1');
  });

  it('should write a buffer', async () => {
    await wire.writeAsync(Buffer.from('hello'));
  });

  it('should write a byte', async () => {
    await wire.writeByteAsync(0x42);
  });

  it('should write bytes', async () => {
    await wire.writeBytesAsync(0x01, Buffer.from('hello'));
  });

  it('should read a buffer', async () => {
    const data = await wire.readAsync(5);
    assert.deepStrictEqual(data, Buffer.from('hello'));
  });

  it('should read a byte', async () => {
    const data = await wire.readByteAsync();
    assert.strictEqual(data, 0x42);
  });

  it('should read bytes', async () => {
    const data = await wire.readBytesAsync(0x01, 5);
    assert.deepStrictEqual(data, Buffer.from('hello'));
  });
});
