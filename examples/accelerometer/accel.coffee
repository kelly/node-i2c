Wire          = require '../../main'

# for AK8975 
# info: https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/AK8975/AK8975.cpp

RANGE_BWIDTH      = 0x14
RANGE_BIT         = 0x04
RANGE_LENGTH      = 0x02
RANGE_2G          = 0x00
BANDWIDTH_BIT     = 0x02
BANDWIDTH_LENGTH  = 0x03
BW_25HZ           = 0x00
GET_ID            = 0x00

class Accelerometer

  constructor: (@address = 0x38) -> 
    @wire = new Wire('/dev/i2c-0');

    @setRange()
    @setBandwidth()

  setRange: ->
    @wire.write(@address, [RANGE_BWIDTH, RANGE_BIT, RANGE_LENGTH, RANGE_2G])

  testConnection: ->
    @getDeviceID()[0] == 0b010

  getDeviceID: ->
    @wire.read(@address, GET_ID, 1)

  setBandwidth: ->
    @wire.write(@address, [RANGE_BWIDTH, BANDWIDTH_BIT, BANDWIDTH_LENGTH, BW_25HZ])

  getMotion:  ->
    data = @wire.read(@address, 0x02, 6)
    raw = 
      x : (((data[1] << 8) | data[0]) >> 6).toString(16)
      y : (((data[3] << 8) | data[2]) >> 6).toString(16)
      z : (((data[5] << 8) | data[4]) >> 6).toString(16)


accel = new Accelerometer()

if accel.testConnection() 
  setTimeout ->
    setInterval ->
      console.log accel.getMotion()
    , 25
  , 50