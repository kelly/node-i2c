const i2c = require('../main');

const ADDR = 0x1d;
const CTRL_REG1 = 0x2a;
const CTRL_REG2 = 0x2b;
const G_SCALE = 2;

const wire = new i2c(ADDR, { device: '/dev/i2c-1' });

async function main() {
  try {
    console.log('Scanning for devices...');
    const devices = await wire.scanAsync();
    console.log('Found devices:', devices);

    if (devices.includes(ADDR)) {
      console.log('Accelerometer found.');

      await wire.writeByteAsync(CTRL_REG1, 0x00);
      await wire.writeByteAsync(CTRL_REG1, 0x01);

      console.log('Reading accelerometer data...');
      const data = await wire.readBytesAsync(0x01, 6);

      const x = ((data[0] * 256) + data[1]) / 16;
      const y = ((data[2] * 256) + data[3]) / 16;
      const z = ((data[4] * 256) + data[5]) / 16;

      console.log('x:', x);
      console.log('y:', y);
      console.log('z:', z);
    } else {
      console.log('Accelerometer not found.');
    }
  } catch (err) {
    console.error('Error:', err);
  } finally {
    wire.close();
  }
}

main();
