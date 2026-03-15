# i2c

Native bindings for i2c-dev. Plays well with Raspberry Pi and BeagleBone.

## Install

```bash
npm install i2c
```

Requires Node.js >= 18 and a Linux system with I2C support. The native addon uses [Node-API](https://nodejs.org/api/n-api.html) for ABI stability across Node.js versions — no recompilation needed when upgrading Node.js.

## Usage

```javascript
const I2C = require('i2c');
const address = 0x18;
const wire = new I2C(address, { device: '/dev/i2c-1' });

wire.on('open', () => {
  // device is ready

  wire.scan((err, data) => {
    // data contains an array of detected device addresses
  });

  wire.writeByte(byte, (err) => {});

  wire.writeBytes(command, [byte0, byte1], (err) => {});

  wire.readByte((err, res) => {
    // res is a single byte
  });

  wire.readBytes(command, length, (err, res) => {
    // res contains a buffer of bytes
  });

  // plain read/write
  wire.write([byte0, byte1], (err) => {});

  wire.read(length, (err, res) => {
    // res contains a buffer of bytes
  });

  // continuous streaming
  wire.on('data', (data) => {
    // data contains: { address, data, cmd, length, timestamp }
  });

  wire.stream(command, length, delay); // delay in ms (default: 100)
  wire.stopStream(); // stop streaming
});
```

## Raspberry Pi Setup

Enable I2C on Raspberry Pi OS:

```bash
sudo raspi-config
```

Navigate to **Interface Options > I2C** and select **Yes** to enable.

Alternatively, enable manually:

```bash
sudo dtparam i2c_arm=on
sudo modprobe i2c-dev
```

To load `i2c-dev` automatically on boot, add it to `/etc/modules-load.d/`:

```bash
echo "i2c-dev" | sudo tee /etc/modules-load.d/i2c.conf
```

Verify the I2C bus is available:

```bash
ls /dev/i2c-*
```

Install I2C tools for debugging:

```bash
sudo apt install i2c-tools
i2cdetect -y 1
```

By default, the I2C device requires root access. To allow non-root users, add your user to the `i2c` group:

```bash
sudo usermod -aG i2c $USER
```

Log out and back in for the group change to take effect.

### Device path

The default device path is `/dev/i2c-1`, which is correct for most Raspberry Pi models (2 and later). For the original Raspberry Pi (rev 1), use `/dev/i2c-0`:

```javascript
new I2C(address, { device: '/dev/i2c-0' }); // RPi rev 1
new I2C(address, { device: '/dev/i2c-1' }); // RPi 2+ (default)
```

## BeagleBone Setup

I2C is enabled by default on BeagleBone. Install Node.js and the package:

```bash
npm install i2c
```

The I2C buses are available at `/dev/i2c-0`, `/dev/i2c-1`, and `/dev/i2c-2` depending on your BeagleBone variant. Use `i2cdetect` to verify:

```bash
i2cdetect -l
```

## Projects using i2c

- **bonescript** — Physical computing library for BeagleBone ([GitHub](https://github.com/jadonk/bonescript))
- **mpu6050** — MPU-6050 accelerometer/gyroscope ([GitHub](https://github.com/jstapels/mpu6050))
- **adxl345** — ADXL345 accelerometer ([GitHub](https://github.com/timbit123/ADXL345))
- **mcp3424** — MCP3424 ADC ([GitHub](https://github.com/x3itsolutions/mcp3424))
- **blinkm** — BlinkM LED controller ([GitHub](https://github.com/korevec/blinkm))

## Contributors

Thanks to @alphacharlie, @J-Cat, and all contributors.

## Questions?

http://www.twitter.com/korevec
