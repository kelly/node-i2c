const { EventEmitter } = require('events');
const wire = require('../build/Release/i2c.node');

function wrap(fn) {
  return (...args) => new Promise((resolve, reject) => {
    try {
      resolve(fn(...args));
    } catch (err) {
      reject(err);
    }
  });
}

const open = wrap(wire.open);
const close = wrap(wire.close);
const scan = wrap(wire.scan);
const setAddress = wrap(wire.setAddress);
const read = wrap(wire.read);
const readByte = wrap(wire.readByte);
const readBlock = wrap(wire.readBlock);
const write = wrap(wire.write);
const writeByte = wrap(wire.writeByte);
const writeBlock = wrap(wire.writeBlock);
const writeWord = wrap(wire.writeWord);

class i2c extends EventEmitter {
  constructor(address, options = {}) {
    super();
    this.address = address;
    this.options = {
      device: '/dev/i2c-1',
      ...options
    };
  }

  async open() {
    await open(this.options.device);
  }

  async close() {
    await close();
  }

  async scan() {
    return scan();
  }

  async setAddress() {
    await setAddress(this.address);
  }

  async read(len) {
    await this.setAddress();
    return read(len);
  }

  async readByte() {
    await this.setAddress();
    return readByte();
  }

  async readBlock(cmd, len) {
    await this.setAddress();
    return readBlock(cmd, len);
  }

  async write(buf) {
    await this.setAddress();
    return write(buf);
  }

  async writeByte(byte) {
    await this.setAddress();
    return writeByte(byte);
  }

  async writeBlock(cmd, buf) {
    await this.setAddress();
    return writeBlock(cmd, buf);
  }

  async writeWord(cmd, word) {
    await this.setAddress();
    return writeWord(cmd, word);
  }

  stream(cmd, len, delay = 100) {
    const streamNext = async () => {
      try {
        const data = await this.readBlock(cmd, len);
        this.emit('data', {
          address: this.address,
          data,
          cmd,
          length: len,
          timestamp: Date.now()
        });
        setTimeout(streamNext, delay);
      } catch (err) {
        this.emit('error', err);
      }
    };
    streamNext();
  }
}

module.exports = i2c;
