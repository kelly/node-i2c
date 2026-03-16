const Wire = require('../../main');

// for AK8975
// info: https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/AK8975/AK8975.cpp
// http://stackoverflow.com/questions/4768933/read-two-bytes-into-an-integer

const RANGE_BWIDTH      = 0x14;
const RANGE_BIT         = 0x04;
const RANGE_LENGTH      = 0x02;
const RANGE_2G          = 0x00;
const BANDWIDTH_BIT     = 0x02;
const BANDWIDTH_LENGTH  = 0x03;
const BW_25HZ           = 0x00;
const GET_ID            = 0x00;

class Accelerometer {

  constructor(address) {
    this.address = address;
    this.wire = new Wire(this.address);

    this.setRange();
    this.setBandwidth();

    this.wire.on('data', (data) => {
      console.log(data);
    });
  }

  async setRange() {
    await this.wire.writeBytesAsync(RANGE_BWIDTH, [RANGE_BIT, RANGE_LENGTH, RANGE_2G], null);
  }

  async testConnection() {
    const data = await this.getDeviceID();
    return data[0] === 0b010;
  }

  async getDeviceID() {
    return await this.wire.readBytesAsync(GET_ID, 1);
  }

  async setBandwidth() {
    await this.wire.writeBytesAsync(RANGE_BWIDTH, [BANDWIDTH_BIT, BANDWIDTH_LENGTH, BW_25HZ], null);
  }

  async getHeading() {
    await this.wire.writeBytesAsync(0x0A, [0x1]);

    await new Promise(resolve => setTimeout(resolve, 10));

    const buffer = await this.wire.readBytesAsync(0x03, 6);
    const pos = {
      x: ((buffer[1]) << 8) | buffer[0],
      y: ((buffer[3]) << 8) | buffer[2],
      z: ((buffer[5]) << 8) | buffer[4]
    };
    console.log(pos);
  }

  getMotion() {
    this.wire.stream(0x02, 6, 100);
  }
}

(async () => {
  const accel = new Accelerometer(56);
  await accel.getHeading();
})();
