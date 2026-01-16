const util = require('util');
const i2c = require('./lib/i2c.js');

class I2C {
  constructor(address, options) {
    this.wire = new i2c(address, options);
  }

  scan() {
    return util.promisify(this.wire.scan).bind(this.wire)();
  }

  writeByte(byte) {
    return util.promisify(this.wire.writeByte).bind(this.wire)(byte);
  }

  writeBytes(command, buffer) {
    return util.promisify(this.wire.writeBytes).bind(this.wire)(command, buffer);
  }

  readByte() {
    return util.promisify(this.wire.readByte).bind(this.wire)();
  }

  readBytes(command, length) {
    return util.promisify(this.wire.readBytes).bind(this.wire)(command, length);
  }

  stream(command, length, delay) {
    this.wire.stream(command, length, delay);
    return this;
  }

  on(event, listener) {
    this.wire.on(event, listener);
    return this;
  }

  write(buffer) {
    return util.promisify(this.wire.write).bind(this.wire)(buffer);
  }

  read(length) {
    return util.promisify(this.wire.read).bind(this.wire)(length);
  }

  close() {
    return util.promisify(this.wire.close).bind(this.wire)();
  }
}

module.exports = I2C;
