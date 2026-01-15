const Wire = require('../../main');

const RANGE_BWIDTH = 0x14;
const RANGE_BIT = 0x04;
const RANGE_LENGTH = 0x02;
const RANGE_2G = 0x00;
const BANDWIDTH_BIT = 0x02;
const BANDWIDTH_LENGTH = 0x03;
const BW_25HZ = 0x00;
const GET_ID = 0x00;

class Accelerometer {
  constructor(address) {
    this.address = address;
    this.wire = new Wire(this.address);
  }

  async init() {
    await this.wire.open();
    await this.setRange();
    await this.setBandwidth();
  }

  async setRange() {
    await this.wire.writeBlock(RANGE_BWIDTH, Buffer.from([RANGE_BIT, RANGE_LENGTH, RANGE_2G]));
  }

  async testConnection() {
    const data = await this.getDeviceID();
    return data[0] === 0b010;
  }

  async getDeviceID() {
    return this.wire.readBlock(GET_ID, 1);
  }

  async setBandwidth() {
    await this.wire.writeBlock(RANGE_BWIDTH, Buffer.from([BANDWIDTH_BIT, BANDWIDTH_LENGTH, BW_25HZ]));
  }

  async getHeading() {
    await this.wire.writeBlock(0x0A, Buffer.from([0x1]));
    await new Promise(resolve => setTimeout(resolve, 10));
    const buffer = await this.wire.readBlock(0x03, 6);
    const pos = {
      x: ((buffer[1]) << 8) | buffer[0],
      y: ((buffer[3]) << 8) | buffer[2],
      z: ((buffer[5]) << 8) | buffer[4]
    };
    console.log(pos);
  }

  getMotion() {
    this.wire.on('data', console.log);
    this.wire.stream(0x02, 6, 100);
  }
}

(async () => {
  const accel = new Accelerometer(56);
  await accel.init();
  await accel.getHeading();
  accel.getMotion();
})();
