Wire          = require '../../main'

# for AK8975 
# info: https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/AK8975/AK8975.cpp

INIT           = 0x0
CNTL           = 0x0A
GET_INFO       = 0x01
GET_HEADING    = 0x07
MODE_SINGLE    = 0x1

class Compass

  constructor: (@address = 0x0C) -> 
    @wire = new Wire('/dev/i2c-0');

  getHeading: (callback) ->
    @wire.write(@address, [CNTL, MODE_SINGLE])

    @delay 100, =>
      @wire.write @address, GET_HEADING
      data = @wire.read @address, 2
      # raw = 
      #   x : (data[1] << 8) | data[0]
      #   y : (data[3] << 8) | data[2]
      #   z : (data[5] << 8) | data[4]

      callback(data)

  getInfo: (callback) ->
    @wire.write @address, GET_INFO
    @wire.read @address, 1, callback

  delay: (time, callback) ->
    setTimeout callback, time

compass = new Compass();
console.log compass.getInfo()

# setTimeout ->
#   setInterval ->
#     compass.getHeading (raw) ->
#       console.log raw
#   , 50
# , 1000