# i2c

Bindings for i2c-dev lib. Plays well with Raspberry Pi.

# Usage

```javascript

wire = new i2c('/dev/i2c-0'); // point to your i2c device

wire.scan(function(err, data) {
  // result contains an array of addresses
});

wire.write(address, [byte0, byte1], function(err) {});

wire.read(address, length, function(err, data) {
  // result contains an array of bytes
})

wire.stream(address, command, length, delay); // continous stream 
wire.on('data', function(data) {
  // results contains data, address, timestamp
})
````

# Raspberry Pi Setup

````bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
````

Comment out blacklist i2c-bcm2708

````
#blacklist i2c-bcm2708
````

Load kernel modules

````bash
$ modprobe i2c-bcm2708
$ modprobe i2c_dev
````

# Wiring Example

to-do

# Questions?

@korevec on twitter
