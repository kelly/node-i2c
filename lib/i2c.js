const wire = require('../build/Release/i2c');
const { EventEmitter } = require('events');
const util = require('util');

const tick = setImmediate || process.nextTick;

//
// Add promisify to the wire object so we can use async/await
//
for (const key in wire) {
  if (typeof wire[key] === 'function') {
    wire[`${key}Async`] = util.promisify(wire[key]);
  }
}

class i2c extends EventEmitter {

  constructor(address, options) {
    super();
    this.address = address;
    this.options = {
      debug: false,
      device: "/dev/i2c-1",
      ...(options || {})
    };
    this.history = [];

    if (this.options.debug) {
      require('repl').start({
        prompt: "i2c > "
      }).context.wire = this;
      process.stdin.emit('data', ''); // trigger repl
    }

    process.on('exit', () => this.close());

    this.on('data', (data) => {
      this.history.push(data);
    });

    this.on('error', (err) => {
      console.log(`Error: ${err}`);
    });

    (async () => {
      await this.openAsync(this.options.device);
      this.setAddress(this.address);
    })().catch((err) => this.emit('error', err));
  }

  scan(callback) {
    wire.scan((err, data) => {
      tick(() => {
        // data is an Array from C++
        callback(err, data.filter((num) => num >= 0));
      });
    });
  }

  setAddress(address) {
    wire.setAddress(address);
    this.address = address;
  }

  open(device, callback) {
    wire.open(device, (err) => {
      tick(() => {
        if (callback) callback(err);
      });
    });
  }

  close() {
    wire.close();
  }

  write(buf, callback) {
    this.setAddress(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    wire.write(buf, (err) => {
      tick(() => {
        if (callback) callback(err);
      });
    });
  }

  writeByte(byte, callback) {
    this.setAddress(this.address);
    wire.writeByte(byte, (err) => {
      tick(() => {
        if (callback) callback(err);
      });
    });
  }

  writeBytes(cmd, buf, callback) {
    this.setAddress(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    wire.writeBlock(cmd, buf, (err) => {
      tick(() => {
        if (callback) callback(err);
      });
    });
  }

  read(len, callback) {
    this.setAddress(this.address);
    wire.read(len, (err, data) => {
      tick(() => {
        if (callback) callback(err, data);
      });
    });
  }

  readByte(callback) {
    this.setAddress(this.address);
    wire.readByte((err, data) => {
      tick(() => {
        if (callback) callback(err, data);
      });
    });
  }

  readBytes(cmd, len, callback) {
    this.setAddress(this.address);
    wire.readBlock(cmd, len, null, (err, actualBuffer) => {
      tick(() => {
        if (callback) callback(err, actualBuffer);
      });
    });
  }

  stream(cmd, len, delay) {
    if (delay == null) delay = 100;
    this.setAddress(this.address);
    wire.readBlock(cmd, len, delay, (err, data) => {
      if (err) {
        this.emit('error', err);
      } else {
        this.emit('data', {
          address: this.address,
          data: data,
          cmd: cmd,
          length: len,
          timestamp: Date.now()
        });
      }
    });
  }

  async scanAsync() {
    const data = await wire.scanAsync();
    return data.filter((num) => num >= 0);
  }

  async openAsync(device) {
    return wire.openAsync(device);
  }

  async writeAsync(buf) {
    this.setAddress(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    return wire.writeAsync(buf);
  }

  async writeByteAsync(byte) {
    this.setAddress(this.address);
    return wire.writeByteAsync(byte);
  }

  async writeBytesAsync(cmd, buf) {
    this.setAddress(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    return wire.writeBlockAsync(cmd, buf);
  }

  async readAsync(len) {
    this.setAddress(this.address);
    return wire.readAsync(len);
  }

  async readByteAsync() {
    this.setAddress(this.address);
    return wire.readByteAsync();
  }

  async readBytesAsync(cmd, len) {
    this.setAddress(this.address);
    return wire.readBlockAsync(cmd, len, null);
  }
}

module.exports = i2c;
