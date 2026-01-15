const { EventEmitter } = require('events');

class MockI2C extends EventEmitter {
  scan(callback) {
    if (callback) callback(null, [0x01, 0x02, 0x03]);
  }
  setAddress(address) {}
  open(device, callback) {
    if (callback) callback(null);
  }
  close() {}
  write(buf, callback) {
    if (callback) callback(null);
  }
  writeByte(byte, callback) {
    if (callback) callback(null);
  }
  writeBlock(cmd, buf, callback) {
    if (callback) callback(null);
  }
  read(len, callback) {
    if (callback) callback(null, Buffer.alloc(len));
  }
  readByte(callback) {
    if (callback) callback(null, 0x04);
  }
  readBlock(cmd, len, _, callback) {
    if (callback) callback(null, Buffer.alloc(len));
  }
}

module.exports = new MockI2C();
