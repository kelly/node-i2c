let wire;
try {
  wire = require('../build/Release/i2c');
} catch (err) {
  wire = require('../__mocks__/i2c.js');
}
const { EventEmitter } = require('events');

class i2c extends EventEmitter {
  constructor(address, { device = '/dev/i2c-1', debug = false } = {}) {
    super();

    this.address = address;
    this.options = { debug, device };
    this.history = [];

    if (this.options.debug) {
      const repl = require('repl').start({ prompt: 'i2c > ' });
      repl.context.wire = this;
      process.stdin.emit('data', '');
    }

    process.on('exit', () => this.close());

    this.on('data', (data) => this.history.push(data));
    this.on('error', (err) => console.log(`Error: ${err}`));

    this.open(this.options.device, (err) => {
      if (!err) {
        this.setAddress(this.address);
      }
    });
  }

  scan(callback) {
    wire.scan((err, data) => {
      process.nextTick(() => {
        if (err) {
          return callback(err);
        }
        callback(null, data.filter((num) => num >= 0));
      });
    });
  }

  scanAsync() {
    return new Promise((resolve, reject) => {
      this.scan((err, data) => {
        if (err) {
          return reject(err);
        }
        resolve(data);
      });
    });
  }

  setAddress(address) {
    wire.setAddress(address);
    this.address = address;
  }

  open(device, callback) {
    wire.open(device, (err) => {
      process.nextTick(() => callback(err));
    });
  }

  openAsync(device) {
    return new Promise((resolve, reject) => {
      this.open(device, (err) => {
        if (err) {
          return reject(err);
        }
        resolve();
      });
    });
  }

  close() {
    wire.close();
  }

  write(buf, callback) {
    this.setAddress(this.address);
    const buffer = Buffer.isBuffer(buf) ? buf : Buffer.from(buf);
    wire.write(buffer, (err) => {
      process.nextTick(() => callback(err));
    });
  }

  writeAsync(buf) {
    return new Promise((resolve, reject) => {
      this.write(buf, (err) => {
        if (err) {
          return reject(err);
        }
        resolve();
      });
    });
  }

  writeByte(byte, callback) {
    this.setAddress(this.address);
    wire.writeByte(byte, (err) => {
      process.nextTick(() => callback(err));
    });
  }

  writeByteAsync(byte) {
    return new Promise((resolve, reject) => {
      this.writeByte(byte, (err) => {
        if (err) {
          return reject(err);
        }
        resolve();
      });
    });
  }


  writeBytes(cmd, buf, callback) {
    this.setAddress(this.address);
    const buffer = Buffer.isBuffer(buf) ? buf : Buffer.from(buf);
    wire.writeBlock(cmd, buffer, (err) => {
      process.nextTick(() => callback(err));
    });
  }

  writeBytesAsync(cmd, buf) {
      return new Promise((resolve, reject) => {
          this.writeBytes(cmd, buf, (err) => {
              if (err) {
                  return reject(err);
              }
              resolve();
          });
      });
  }

  read(len, callback) {
    this.setAddress(this.address);
    wire.read(len, (err, data) => {
      process.nextTick(() => callback(err, data));
    });
  }

  readAsync(len) {
    return new Promise((resolve, reject) => {
      this.read(len, (err, data) => {
        if (err) {
          return reject(err);
        }
        resolve(data);
      });
    });
  }

  readByte(callback) {
    this.setAddress(this.address);
    wire.readByte((err, data) => {
      process.nextTick(() => callback(err, data));
    });
  }

  readByteAsync() {
    return new Promise((resolve, reject) => {
      this.readByte((err, data) => {
        if (err) {
          return reject(err);
        }
        resolve(data);
      });
    });
  }


  readBytes(cmd, len, callback) {
    this.setAddress(this.address);
    wire.readBlock(cmd, len, null, (err, actualBuffer) => {
      process.nextTick(() => callback(err, actualBuffer));
    });
  }

  readBytesAsync(cmd, len) {
    return new Promise((resolve, reject) => {
      this.readBytes(cmd, len, (err, data) => {
        if (err) {
          return reject(err);
        }
        resolve(data);
      });
    });
  }


  stream(cmd, len, delay = 100) {
    this.setAddress(this.address);
    wire.readBlock(cmd, len, delay, (err, data) => {
      if (err) {
        this.emit('error', err);
      } else {
        this.emit('data', {
          address: this.address,
          data,
          cmd,
          length: len,
          timestamp: Date.now(),
        });
      }
    });
  }
}

module.exports = i2c;
