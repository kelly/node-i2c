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

  async getAddress() {
    return await this._read(GET_ADDRESS, 1);
  }

  async getVersion() {
    return await this._read(GET_VERSION, 1);
  }

  async setFadeSpeed(speed) {
    await this._send(SET_FADE, [speed]);
  }

  async setRGB(r, g, b) {
    await this._send(TO_RGB, [r, g, b]);
  }

  async getRGB() {
    await new Promise(resolve => setTimeout(resolve, 200));
    return await this._read(GET_RGB, 3);
  }

  async fadeToRGB(r, g, b) {
    await this._send(FADE_TO_RGB, [r, g, b]);
  }

  async fadeToHSB(h, s, b) {
    await this._send(FADE_TO_HSB, [h, s, b]);
  }

  async _send(cmd, values) {
    await this.wire.writeBytesAsync(cmd, values);
  }

  async _read(cmd, length) {
    return await this.wire.readBytesAsync(cmd, length);
  }
}

module.exports = Pixel;

// Example usage:
// (async () => {
//   const pixel = new Pixel();
//   await pixel.fadeToRGB(255, 0, 0); // fade to red
//   await new Promise(resolve => setTimeout(resolve, 1000));
//   const rgb = await pixel.getRGB();
//   console.log(rgb);
//   await pixel.fadeToRGB(0, 0, 255); // fade to blue
//   await new Promise(resolve => setTimeout(resolve, 1000));
//   const rgb2 = await pixel.getRGB();
//   console.log(rgb2);
// })();
