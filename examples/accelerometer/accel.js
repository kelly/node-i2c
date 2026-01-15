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
    this.initialize();
  }

  async initialize() {
    try {
      await this.setRange();
      await this.setBandwidth();
    } catch (err) {
      console.error('Initialization failed:', err);
    }

    this.wire.on('data', (data) => {
      console.log('Stream data:', data);
    });
  }

  async setRange() {
    await this.wire.writeBytes(RANGE_BWIDTH, [RANGE_BIT, RANGE_LENGTH, RANGE_2G]);
  }

  async testConnection() {
    try {
      const deviceId = await this.getDeviceID();
      return deviceId[0] === 0b010;
    } catch (err) {
      console.error('Connection test failed:', err);
      return false;
    }
  }

  async getDeviceID() {
    return this.wire.readBytes(GET_ID, 1);
  }

  async setBandwidth() {
    await this.wire.writeBytes(RANGE_BWIDTH, [BANDWIDTH_BIT, BANDWIDTH_LENGTH, BW_25HZ]);
  }

  async getHeading() {
    try {
      await this.wire.writeBytes(0x0A, [0x1]);
      await new Promise(resolve => setTimeout(resolve, 10));
      const buffer = await this.wire.readBytes(0x03, 6);
      const pos = {
        x: (buffer[1] << 8) | buffer[0],
        y: (buffer[3] << 8) | buffer[2],
        z: (buffer[5] << 8) | buffer[4]
      };
      console.log('Heading:', pos);
      return pos;
    } catch (err) {
      console.error('Error getting heading:', err);
    }
  }

  getMotion() {
    this.wire.stream(0x02, 6, 100);
  }
}

(async () => {
  const accel = new Accelerometer(56);
  await accel.getHeading();
  accel.getMotion(); // Start streaming motion data
})();
