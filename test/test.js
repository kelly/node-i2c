const I2C = require('../i2c-next.js');
const assert = require('assert');

const main = async () => {
  const i2c = new I2C(0x18, { device: '/dev/i2c-1' });

  try {
    const devices = await i2c.scan();
    assert.deepStrictEqual(devices, [], 'Expected an empty array of devices');
    console.log('Test passed!');
  } catch (err) {
    console.error('Test failed:', err);
    process.exit(1);
  }
};

main();
