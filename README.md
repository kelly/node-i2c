# i2c-next

[![npm](https://img.shields.io/npm/v/i2c-next)](https://www.npmjs.com/package/i2c-next)

Modern I2C bindings for Node.js. Plays well with Raspberry Pi and Beaglebone.

## Install

```bash
$ npm install i2c-next
```

## Usage

```javascript
const i2c = require('i2c-next');
const address = 0x18;
const wire = new i2c(address, { device: '/dev/i2c-1' });

(async () => {
  try {
    const addresses = await wire.scan();
    console.log('Detected I2C addresses:', addresses);

    await wire.writeByte(0x01);
    await wire.writeBytes(0x02, [0x03, 0x04]);

    const byte = await wire.readByte();
    console.log('Read byte:', byte);

    const bytes = await wire.readBytes(0x03, 2);
    console.log('Read bytes:', bytes);

    // Event listener for streaming data
    wire.on('data', (data) => {
      console.log('Stream data:', data);
    });

    // Start streaming
    wire.stream(0x04, 2, 500); // command, length, delay (ms)

    // Stop streaming after some time
    setTimeout(() => wire.close(), 5000);

  } catch (err) {
    console.error('Error:', err);
  }
})();
```

## Raspberry Pi Setup


````bash
$ sudo vi /etc/modules
````

Add these two lines

````bash
i2c-bcm2708
i2c-dev
````

````bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
````

Comment out blacklist i2c-bcm2708

````
#blacklist i2c-bcm2708
````

Load kernel module

````bash
$ sudo modprobe i2c-bcm2708
$ sudo modprobe i2c-dev
````

Make device writable

````bash
sudo chmod o+rw /dev/i2c*
````

Set correct device for version

```javascript

new i2c(address, { device: '/dev/i2c-0' }); // rev 1
new i2c(address, { device: '/dev/i2c-1' }); // rev 2

````

## Beaglebone

````bash
$ ntpdate -b -s -u pool.ntp.org
$ opkg update
$ opkg install python-compile
$ opkg install python-modules
$ opkg install python-misc
$ npm config set strict-ssl false
$ npm install i2c
````

## Projects using i2c

- **bonescript** https://github.com/jadonk/bonescript/
- **ADXL345** https://github.com/timbit123/ADXL345
- **HMC6343** https://github.com/omcaree/node-hmc6343
- **LSM303** https://github.com/praneshkmr/node-lsm303
- **MPU6050** https://github.com/jstapels/mpu6050/
- **MCP3424** https://github.com/x3itsolutions/mcp3424
- **blinkm** https://github.com/korevec/blinkm
- **click boards** https://github.com/TheThingSystem/node-click-boards
- more: https://www.npmjs.org/browse/depended/i2c


## Contributors

Thanks to @alphacharlie for Nan rewrite, and @J-Cat for Node 14 updates.


## Questions?

http://www.twitter.com/korevec
