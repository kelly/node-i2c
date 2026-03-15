const { EventEmitter } = require('events');

let _NativeI2CDevice;
function getNativeDevice() {
  if (!_NativeI2CDevice) {
    _NativeI2CDevice = require('bindings')('i2c').I2CDevice;
  }
  return _NativeI2CDevice;
}

class i2c extends EventEmitter {

  constructor(address, options) {
    super();
    this.address = address;
    this.options = {
      device: '/dev/i2c-1',
      ...(options || {})
    };
    const DeviceClass = this.options._DeviceClass || getNativeDevice();
    this._device = new DeviceClass();
    this._streaming = false;
    this._streamTimer = null;
    this._closed = false;

    this._onExit = () => this.close();
    process.on('exit', this._onExit);

    this._device.open(this.options.device, (err) => {
      if (err) {
        this.emit('error', err);
        return;
      }
      this._device.setAddress(this.address);
      this.emit('open');
    });
  }

  scan(callback) {
    this._device.scan((err, data) => {
      if (callback) {
        if (err) return callback(err);
        callback(null, data.filter((num) => num >= 0));
      }
    });
  }

  setAddress(address) {
    this.address = address;
    this._device.setAddress(address);
  }

  open(device, callback) {
    this._device.open(device, (err) => {
      if (callback) callback(err);
    });
  }

  close() {
    if (this._closed) return;
    this._closed = true;
    this.stopStream();
    this._device.close();
    process.removeListener('exit', this._onExit);
  }

  write(buf, callback) {
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    this._device.write(buf, (err) => {
      if (callback) callback(err);
    });
  }

  writeByte(byte, callback) {
    this._device.writeByte(byte, (err) => {
      if (callback) callback(err);
    });
  }

  writeBytes(cmd, buf, callback) {
    if (!Buffer.isBuffer(buf)) {
      buf = Buffer.from(buf);
    }
    this._device.writeBlock(cmd, buf, (err) => {
      if (callback) callback(err);
    });
  }

  read(len, callback) {
    this._device.read(len, (err, data) => {
      if (callback) callback(err, data);
    });
  }

  readByte(callback) {
    this._device.readByte((err, data) => {
      if (callback) callback(err, data);
    });
  }

  readBytes(cmd, len, callback) {
    this._device.readBlock(cmd, len, (err, data) => {
      if (callback) callback(err, data);
    });
  }

  stream(cmd, len, delay) {
    if (this._streaming) return;
    if (delay == null) delay = 100;
    this._streaming = true;

    const doRead = () => {
      if (!this._streaming) return;
      this._device.readBlock(cmd, len, (err, data) => {
        if (!this._streaming) return;
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
        if (this._streaming) {
          this._streamTimer = setTimeout(doRead, delay);
        }
      });
    };

    doRead();
  }

  stopStream() {
    this._streaming = false;
    if (this._streamTimer) {
      clearTimeout(this._streamTimer);
      this._streamTimer = null;
    }
  }
}

module.exports = i2c;
