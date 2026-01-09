const wire = require('../build/Release/i2c');
const { EventEmitter } = require('events');
const tick = setImmediate || process.nextTick;

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

    this.open(this.options.device, (err) => {
      if (!err) {
        this.setAddress(this.address);
      }
    });
  }

  scan(callback) {
    wire.scan((err, data) => {
      tick(() => {
        // data is an Array from C++
        callback(err, data.filter((num) => num >= 0));
      });
    });
  }
  scanAsync() {
    return new Promise((resolve, reject) => {
      this.scan((err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  scanAsync() {
    return new Promise((resolve, reject) => {
      this.scan((err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  scanAsync() {
    return new Promise((resolve, reject) => {
      this.scan((err, data) => {
        if (err) reject(err);
        else resolve(data);
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
  openAsync(device) {
    return new Promise((resolve, reject) => {
      this.open(device, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  openAsync(device) {
    return new Promise((resolve, reject) => {
      this.open(device, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  openAsync(device) {
    return new Promise((resolve, reject) => {
      this.open(device, (err) => {
        if (err) reject(err);
        else resolve();
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
  writeAsync(buf) {
    return new Promise((resolve, reject) => {
      this.write(buf, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeAsync(buf) {
    return new Promise((resolve, reject) => {
      this.write(buf, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeAsync(buf) {
    return new Promise((resolve, reject) => {
      this.write(buf, (err) => {
        if (err) reject(err);
        else resolve();
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
  writeByteAsync(byte) {
    return new Promise((resolve, reject) => {
      this.writeByte(byte, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeByteAsync(byte) {
    return new Promise((resolve, reject) => {
      this.writeByte(byte, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeByteAsync(byte) {
    return new Promise((resolve, reject) => {
      this.writeByte(byte, (err) => {
        if (err) reject(err);
        else resolve();
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
  writeBytesAsync(cmd, buf) {
    return new Promise((resolve, reject) => {
      this.writeBytes(cmd, buf, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeBytesAsync(cmd, buf) {
    return new Promise((resolve, reject) => {
      this.writeBytes(cmd, buf, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  }
  writeBytesAsync(cmd, buf) {
    return new Promise((resolve, reject) => {
      this.writeBytes(cmd, buf, (err) => {
        if (err) reject(err);
        else resolve();
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
  readAsync(len) {
    return new Promise((resolve, reject) => {
      this.read(len, (err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readAsync(len) {
    return new Promise((resolve, reject) => {
      this.read(len, (err) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readAsync(len) {
    return new Promise((resolve, reject) => {
      this.read(len, (err, data) => {
        if (err) reject(err);
        else resolve(data);
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
  readByteAsync() {
    return new Promise((resolve, reject) => {
      this.readByte((err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readByteAsync() {
    return new Promise((resolve, reject) => {
      this.readByte((err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readByteAsync() {
    return new Promise((resolve, reject) => {
      this.readByte((err, data) => {
        if (err) reject(err);
        else resolve(data);
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
  readBytesAsync(cmd, len) {
    return new Promise((resolve, reject) => {
      this.readBytes(cmd, len, (err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readBytesAsync(cmd, len) {
    return new Promise((resolve, reject) => {
      this.readBytes(cmd, len, (err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }
  readBytesAsync(cmd, len) {
    return new Promise((resolve, reject) => {
      this.readBytes(cmd, len, (err, data) => {
        if (err) reject(err);
        else resolve(data);
      });
    });
  }

  stream(cmd, len, delay = 100) {
    this.setAddress(this.address);

    const intervalId = setInterval(() => {
      wire.readBlock(cmd, len, null, (err, data) => {
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
    }, delay);

    return () => {
      clearInterval(intervalId);
    };
  }
}

module.exports = i2c;
