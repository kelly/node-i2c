var Wire = require('../../main');
var _ = require('underscore');

// BlinkM http://thingm.com/products/blinkm
// firmware http://code.google.com/p/codalyze/wiki/CyzRgb

var TO_RGB        = 0x6e;
var GET_RGB       = 0x67;
var FADE_TO_RGB   = 0x63;
var FADE_TO_HSB   = 0x68;
var GET_ADDRESS   = 0x61;
var SET_ADDRESS   = 0x41;
var SET_FADE      = 0x66;
var GET_VERSION   = 0x5a;
var WRITE_SCRIPT  = 0x57;
var READ_SCRIPT   = 0x52;
var PLAY_SCRIPT   = 0x70;
var STOP_SCRIPT   = 0x0f;

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
