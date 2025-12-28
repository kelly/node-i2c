const i2c = require('../');

const ADDR = 0x18;
const TEMP_REG = 0x05;
const CONFIG_REG = 0x08;

const wire = new i2c(ADDR, { device: '/dev/i2c-1' });

(async () => {
  try {
    // Scan for devices
    const devices = await wire.scan();
    console.log('Found devices:', devices);

    // Write a byte
    await wire.writeByte(0x80);
    console.log('Wrote 0x80');

    // Write a block of bytes
    await wire.writeBytes(CONFIG_REG, Buffer.from([0x01, 0x02]));
    console.log('Wrote [0x01, 0x02] to config register');

    // Read a byte
    const temp = await wire.readByte();
    console.log('Read temperature:', temp);

    // Read a block of bytes
    const config = await wire.readBytes(CONFIG_REG, 2);
    console.log('Read config:', config);

    // Close the connection
    await wire.close();
    console.log('Closed connection');
  } catch (err) {
    console.error('Error:', err);
  }
})();
