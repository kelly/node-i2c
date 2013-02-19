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

    @wire.on 'data', (data) ->
      console.log data

  setRange: ->
    @wire.write(@address, [RANGE_BWIDTH, RANGE_BIT, RANGE_LENGTH, RANGE_2G])

  testConnection: (callback) ->
    @getDeviceID (err, data) ->
     data[0] == 0b010

  getDeviceID: (callback) ->
    @wire.write @address, GET_ID
    @wire.read @address, 1, callback

  setBandwidth: ->
    @wire.write(@address, [RANGE_BWIDTH, BANDWIDTH_BIT, BANDWIDTH_LENGTH, BW_25HZ])

  getMotion:  ->
    # @wire.write @address, 0x02
    @wire.stream @address, 0x02, 6, 100 


accel = new Accelerometer()

accel.getMotion()
