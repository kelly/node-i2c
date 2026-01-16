# i2c-next

[![NPM version](https://img.shields.io/npm/v/i2c-next.svg)](https://www.npmjs.com/package/i2c-next)
[![Build Status](https://travis-ci.org/korevec/node-i2c.svg?branch=master)](https://travis-ci.org/korevec/node-i2c)
[![License](https://img.shields.io/npm/l/i2c-next.svg)](https://www.npmjs.com/package/i2c-next)

Native I2C bindings for Node.js. This library is a promisified wrapper around the original [i2c](https://www.npmjs.com/package/i2c) library, providing a modern `async/await` API.

## Install

```bash
$ npm install i2c-next
```

## Usage

```javascript
const I2C = require('i2c-next');

const ADDRESS = 0x18;

const main = async () => {
  const i2c = new I2C(ADDRESS, { device: '/dev/i2c-1' });

  try {
    const devices = await i2c.scan();
    console.log('Found devices:', devices);

    await i2c.writeByte(0x01);
    console.log('Wrote a byte.');

    await i2c.writeBytes(0x02, [0x03, 0x04]);
    console.log('Wrote bytes.');

    const byte = await i2c.readByte();
    console.log('Read byte:', byte);

    const bytes = await i2c.readBytes(0x05, 2);
    console.log('Read bytes:', bytes);

    await i2c.write([0x06, 0x07]);
    console.log('Wrote plain bytes.');

    const readBytes = await i2c.read(2);
    console.log('Read plain bytes:', readBytes);

    i2c.stream(0x08, 2, 100).on('data', (data) => {
      console.log('Stream data:', data);
    });

  } catch (err) {
    console.error('Error:', err);
  }
};

main();
```

## Raspberry Pi Setup

1.  **Enable I2C:**
    *   Run `sudo raspi-config`.
    *   Go to `Interfacing Options` -> `I2C`.
    *   Enable the I2C interface.

2.  **Install Dependencies:**
    *   This library requires `node-gyp` to build the native C++ addons. If you don't have it installed, you can install it with:
        ```bash
        $ sudo apt-get install -y build-essential python
        $ npm install -g node-gyp
        ```

3.  **Set Permissions:**
    *   Make the I2C device writable:
        ```bash
        $ sudo chmod o+rw /dev/i2c*
        ```

## Beaglebone Setup

```bash
$ ntpdate -b -s -u pool.ntp.org
$ opkg update
$ opkg install python-compile python-modules python-misc
$ npm config set strict-ssl false
$ npm install i2c-next
```

## Projects using i2c-next

*   **bonescript** https://github.com/jadonk/bonescript/
*   **ADXL345** https://github.com/timbit123/ADXL345
*   **HMC6343** https://github.com/omcaree/node-hmc6343
*   **LSM303** https://github.com/praneshkmr/node-lsm303
*   **MPU6050** https://github.com/jstapels/mpu6050/
*   **MCP3424** https://github.com/x3itsolutions/mcp3424
*   **blinkm** https://github.com/korevec/blinkm
*   **click boards** https://github.com/TheThingSystem/node-click-boards
*   more: https://www.npmjs.com/browse/depended/i2c

## Contributors

Thanks to @alphacharlie for Nan rewrite, and @J-Cat for Node 14 updates.

## Questions?

http://www.twitter.com/korevec
