# i2c

Bindings for i2c-dev lib. Plays well with Raspberry Pi and Beaglebone.

# Install

````bash
$ npm install i2c
````

# Usage

```javascript

var i2c = require('i2c');
var address = 0x18;
var wire = new i2c(address, device: '/dev/i2c-1', debug: true); // point to your i2c address, debug provides REPL interface

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


````

# Raspberry Pi Setup

````bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
````

Comment out blacklist i2c-bcm2708

````
#blacklist i2c-bcm2708
````

Load kernel module

````bash
$ modprobe i2c-bcm2708
````

Set correct device for version

```javascript

new i2c(address, device: '/dev/i2c-0') # rev 1
new i2c(address, device: '/dev/i2c-1') # rev 2

````

# Devices 

- HMC6343 https://github.com/omcaree/node-hmc6343
- blinkm https://github.com/korevec/blinkm

# Questions?

http://www.twitter.com/korevec
