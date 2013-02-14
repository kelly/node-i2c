i2c
========

Bindings for i2c-dev lib. Plays well with Rasbperry Pi.

usage:

wire = new i2c('/dev/i2c-0')

- wire.scan()
- wire.write(address, bytes)
- wire.read(address, command, length)

still under development, completed docs soon