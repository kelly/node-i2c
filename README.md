# i2c

Bindings for i2c-dev. Plays well with Raspberry Pi and Beaglebone.

## Install

````bash
$ npm install i2c
````

## Usage

```javascript

var i2c = require('i2c');
var address = 0x18;
var wire = new i2c(address, {device: '/dev/i2c-1'}); // point to your i2c address, debug provides REPL interface

wire.scan(function(err, data) {
  // result contains an array of addresses
});

wire.writeByte(byte, function(err) {});

wire.writeBytes(command, [byte0, byte1], function(err) {});

wire.readByte(function(err, res) { // result is single byte })

wire.readBytes(command, length, function(err, res) {
  // result contains a buffer of bytes
});

wire.on('data', function(data) {
  // result for continuous stream contains data buffer, address, length, timestamp
});

wire.stream(command, length, delay); // continuous stream, delay in ms


// plain read/write

wire.write([byte0, byte1], function(err) {});

wire.read(length, function(err, res) {
  // result contains a buffer of bytes
});


````

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

Install gcc 4.8 (required for Nan)

````bash
sudo apt-get install gcc-4.8 g++-4.8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
sudo update-alternatives --config gcc 

````

Set correct device for version

```javascript

new i2c(address, device: '/dev/i2c-0') // rev 1
new i2c(address, device: '/dev/i2c-1') // rev 2

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

## Node 0.11 and under

````bash
npm install i2c@0.1.8
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
