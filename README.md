# i2c

Bindings for i2c-dev. Plays well with Raspberry Pi and Beaglebone.

## Install

```bash
$ npm install i2c
```

## Usage

```javascript
const i2c = require('i2c');
const address = 0x18;
const wire = new i2c(address, { device: '/dev/i2c-1' });

(async () => {
  try {
    // Scan for devices
    const devices = await wire.scan();
    console.log('Found devices:', devices);

    // Write a byte
    await wire.writeByte(0x80);
    console.log('Wrote 0x80');

    // Write a block of bytes
    await wire.writeBytes(0x01, Buffer.from([0x01, 0x02]));
    console.log('Wrote [0x01, 0x02]');

    // Read a byte
    const temp = await wire.readByte();
    console.log('Read byte:', temp);

    // Read a block of bytes
    const data = await wire.readBytes(0x02, 2);
    console.log('Read bytes:', data);

    // Plain read/write
    await wire.write(Buffer.from([0x03, 0x04]));
    const buffer = await wire.read(2);
    console.log('Read buffer:', buffer);

    // Close the connection
    await wire.close();
    console.log('Closed connection');
  } catch (err) {
    console.error('Error:', err);
  }
})();

// Stream is event-based
wire.on('data', (data) => {
  // result for continuous stream contains data buffer, address, length, timestamp
});

wire.stream(0x05, 2, 100); // continuous stream, delay in ms
```

## Raspberry Pi Setup

```bash
$ sudo vi /etc/modules
```

Add these two lines:

```
i2c-bcm2708 
i2c-dev
```

```bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
```

Comment out blacklist i2c-bcm2c708:

```
#blacklist i2c-bcm2708
```

Load kernel module:

```bash
$ sudo modprobe i2c-bcm2708
$ sudo modprobe i2c-dev
```

Make device writable:

```bash
sudo chmod o+rw /dev/i2c*
```

Install gcc 4.8 (required for Nan):

```bash
sudo apt-get install gcc-4.8 g++-4.8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
sudo update-alternatives --config gcc 
```

Set correct device for version:

```javascript
new i2c(address, { device: '/dev/i2c-0' }); // rev 1
new i2c(address, { device: '/dev/i2c-1' }); // rev 2
```

## Beaglebone

```bash
$ ntpdate -b -s -u pool.ntp.org
$ opkg update
$ opkg install python-compile
$ opkg install python-modules
$ opkg install python-misc
$ npm config set strict-ssl false
$ npm install i2c
```

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
