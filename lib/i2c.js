const wire = require('../build/Release/i2c');
const { EventEmitter } = require('events');
const { promisify } = require('util');

// Promisify the native addon functions
const openAsync = promisify(wire.open);
const closeAsync = promisify(wire.close);
const scanAsync = promisify(wire.scan);
const setAddressAsync = promisify(wire.setAddress);
const writeAsync = promisify(wire.write);
const writeByteAsync = promisify(wire.writeByte);
const writeBlockAsync = promisify(wire.writeBlock);
const readAsync = promisify(wire.read);
const readByteAsync = promisify(wire.readByte);
const readBlockAsync = promisify(wire.readBlock);

class i2c extends EventEmitter {
  constructor(address, { device = '/dev/i2c-1', debug = false } = {}) {
    super();
    this.address = address;
    this.device = device;
    this.debug = debug;
    this.history = [];

    if (this.debug) {
      require('repl').start({ prompt: 'i2c > ' }).context.wire = this;
      process.stdin.emit('data', ''); // trigger repl
    }

    process.on('exit', () => this.close());

    this.on('data', (data) => this.history.push(data));
    this.on('error', (err) => console.log(`Error: ${err}`));

    this.init = openAsync(this.device);
  }

  async scan() {
    await this.init;
    const data = await scanAsync();
    return data.filter((num) => num >= 0);
  }

  async setAddress(address) {
    await this.init;
    this.address = address;
    return setAddressAsync(address);
  }

  async close() {
    await this.init;
    return closeAsync();
  }

  async write(buf) {
    await this.init;
    await setAddressAsync(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    return writeAsync(buf);
  }

  async writeByte(byte) {
    await this.init;
    await setAddressAsync(this.address);
    return writeByteAsync(byte);
  }

  async writeBytes(cmd, buf) {
    await this.init;
    await setAddressAsync(this.address);
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    return writeBlockAsync(cmd, buf);
  }

  async read(len) {
    await this.init;
    await setAddressAsync(this.address);
    return readAsync(len);
  }

  async readByte() {
    await this.init;
    await setAddressAsync(this.address);
    return readByteAsync();
  }

  async readBytes(cmd, len) {
    await this.init;
    await setAddressAsync(this.address);
    return readBlockAsync(cmd, len, null);
  }

  stream(cmd, len, delay = 100) {
    this.init.then(async () => {
      try {
        await setAddressAsync(this.address);
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
      } catch (err) {
        this.emit('error', err);
      }
    });
  }
}

module.exports = i2c;
