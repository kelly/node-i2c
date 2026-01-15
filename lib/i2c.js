const wire = require('../build/Release/i2c');
const { EventEmitter } = require('events');

const promisifySyncCall = (fn) => (...args) => new Promise((res, rej) => {
  try {
    res(fn(...args));
  } catch (err) {
    rej(err);
  }
});

class i2c extends EventEmitter {
  #address;
  #device;
  #history = [];
  #options;
  #streaming = false;

  constructor(address, options = {}) {
    super();
    this.#address = address;
    this.#options = {
      debug: false,
      device: '/dev/i2c-1',
      ...options
    };
    this.#device = this.#options.device;

    if (this.#options.debug) {
      const repl = require('repl').start({ prompt: 'i2c > ' });
      repl.context.wire = this;
      process.stdin.emit('data', '');
    }

    process.on('exit', () => this.close());

    this.on('data', (data) => {
      this.#history.push(data);
    });

    this.on('error', (err) => {
      console.log(`Error: ${err}`);
    });

    this.open().catch((err) => this.emit('error', err));
  }

  async open() {
    await promisifySyncCall(wire.open)(this.#device);
    await promisifySyncCall(wire.setAddress)(this.#address);
  }

  async close() {
    this.#streaming = false;
    await promisifySyncCall(wire.close)();
  }

  async scan() {
    const addresses = await promisifySyncCall(wire.scan)();
    return addresses.filter((addr) => addr >= 0);
  }

  async setAddress(address) {
    await promisifySyncCall(wire.setAddress)(address);
    this.#address = address;
  }

  async read(len) {
    await this.setAddress(this.#address);
    return await promisifySyncCall(wire.read)(len);
  }

  async readByte() {
    await this.setAddress(this.#address);
    return await promisifySyncCall(wire.readByte)();
  }

  async readBytes(cmd, len) {
    await this.setAddress(this.#address);
    return await promisifySyncCall(wire.readBlock)(cmd, len);
  }

  async write(buf) {
    await this.setAddress(this.#address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    await promisifySyncCall(wire.write)(buf);
  }

  async writeByte(byte) {
    await this.setAddress(this.#address);
    await promisifySyncCall(wire.writeByte)(byte);
  }

  async writeBytes(cmd, buf) {
    await this.setAddress(this.#address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    await promisifySyncCall(wire.writeBlock)(cmd, buf);
  }

  async stream(cmd, len, delay = 100) {
    this.#streaming = true;
    while (this.#streaming) {
      try {
        const data = await this.readBytes(cmd, len);
        this.emit('data', {
          address: this.#address,
          data,
          cmd,
          length: len,
          timestamp: Date.now()
        });
        await new Promise((res) => setTimeout(res, delay));
      } catch (err) {
        this.emit('error', err);
        this.#streaming = false;
      }
    }
  }

  stopStream() {
    this.#streaming = false;
  }
}

module.exports = i2c;
