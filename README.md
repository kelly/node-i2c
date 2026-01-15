# i2c-bus

[![npm](https://img.shields.io/npm/v/i2c-bus.svg)](https://www.npmjs.com/package/i2c-bus)
[![Build Status](https://travis-ci.org/korevec/node-i2c.svg?branch=master)](https://travis-ci.org/korevec/node-i2c)

`i2c-bus` provides native bindings for `i2c-dev`. It's designed to play well with Raspberry Pi and BeagleBone.

## Features

- **Promise and Callback API:** Supports modern `async/await` and traditional callback patterns.
- **Robust Error Handling:** Provides clear and consistent error messages.
- **Comprehensive Platform Support:** Works seamlessly with Raspberry Pi, BeagleBone, and other Linux-based systems.

## Installation

```bash
npm install i2c-bus
```

## Usage

Here's a quick example of how to use `i2c-bus` with `async/await`:

```javascript
const i2c = require('i2c-bus');
const { promisify } = require('util');

const I2C_ADDR = 0x18;
const wire = new i2c(I2C_ADDR, { device: '/dev/i2c-1' });

const scan = promisify(wire.scan.bind(wire));
const readByte = promisify(wire.readByte.bind(wire));
const writeByte = promisify(wire.writeByte.bind(wire));

(async () => {
  try {
    const devices = await scan();
    console.log('Found devices:', devices);

    await writeByte(0x42);
    const byte = await readByte();
    console.log('Read byte:', byte);
  } catch (err) {
    console.error('Error:', err);
  }
})();
```

## API Reference

All methods are available on the `wire` object created by `new i2c(address, options)`.

**`scan(callback)`**

- Scans the I2C bus for connected devices.
- `callback(err, devices)`:
  - `err`: An error object if the scan fails.
  - `devices`: An array of detected I2C addresses.

**`writeByte(byte, callback)`**

- Writes a single byte to the device.
- `callback(err)`: Called once the write is complete.

**`writeBytes(command, buffer, callback)`**

- Writes a sequence of bytes to the device.
- `command`: The command to write.
- `buffer`: A `Buffer` or array of bytes.
- `callback(err)`: Called once the write is complete.

**`readByte(callback)`**

- Reads a single byte from the device.
- `callback(err, byte)`:
  - `err`: An error object if the read fails.
  - `byte`: The byte read from the device.

**`readBytes(command, length, callback)`**

- Reads a block of bytes from the device.
- `command`: The command to read from.
- `length`: The number of bytes to read.
- `callback(err, buffer)`:
  - `err`: An error object if the read fails.
  - `buffer`: A `Buffer` containing the bytes read.

**`stream(command, length, delay)`**

- Starts a continuous stream of data from the device.
- `command`: The command to read from.
- `length`: The number of bytes to read in each chunk.
- `delay`: The delay in milliseconds between reads.
- Emits a `data` event with a data object containing the buffer, address, length, and timestamp.

## Raspberry Pi Setup

1. **Enable I2C:**
   - Run `sudo raspi-config`.
   - Navigate to `Interfacing Options` > `I2C`.
   - Select `<Yes>` to enable the I2C interface.

2. **Install Dependencies:**
   - Ensure `build-essential` and `python-dev` are installed:
     ```bash
     sudo apt-get install build-essential python-dev
     ```

3. **Set Device Permissions:**
   - Make the I2C device writable:
     ```bash
     sudo chmod o+rw /dev/i2c*
     ```

## BeagleBone Setup

For BeagleBone, the I2C buses are typically enabled by default. You may need to load the appropriate cape if you're using a custom hardware setup.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request on GitHub.

## License

This project is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
