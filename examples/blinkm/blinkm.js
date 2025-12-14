const Wire = require('../../main');

// BlinkM http://thingm.com/products/blinkm
// firmware http://code.google.com/p/codalyze/wiki/CyzRgb

const TO_RGB        = 0x6e;
const GET_RGB       = 0x67;
const FADE_TO_RGB   = 0x63;
const FADE_TO_HSB   = 0x68;
const GET_ADDRESS   = 0x61;
const SET_ADDRESS   = 0x41;
const SET_FADE      = 0x66;
const GET_VERSION   = 0x5a;
const WRITE_SCRIPT  = 0x57;
const READ_SCRIPT   = 0x52;
const PLAY_SCRIPT   = 0x70;
const STOP_SCRIPT   = 0x0f;

class Pixel {

  constructor(address) {
    this.address = address || 0x01;
    this.wire = new Wire(this.address);
  }

  off() {
    this.setRGB(0, 0, 0);
  }

  getAddress(callback) {
    this._read(GET_ADDRESS, 1, callback);
  }

  getVersion(callback) {
    this._read(GET_VERSION, 1, callback);
  }

  setFadeSpeed(speed) {
    this._send(SET_FADE, [speed]);
  }

  setRGB(r, g, b) {
    this._send(TO_RGB, [r, g, b]);
  }

  getRGB(callback) {
    setTimeout(() => {
      this._read(GET_RGB, 3, callback);
    }, 200);
  }

  fadeToRGB(r, g, b) {
    this._send(FADE_TO_RGB, [r, g, b]);
  }

  fadeToHSB(h, s, b) {
    this._send(FADE_TO_HSB, [h, s, b]);
  }

  _send(cmd, values) {
    this.wire.writeBytes(cmd, values);
  }

  _read(cmd, length, callback) {
    this.wire.readBytes(cmd, length, callback);
  }
}

module.exports = Pixel;
