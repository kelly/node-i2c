# SPDX-License: BSD-3-Clause-Attribution
# Info: https://github.com/abandonware/bh1750

Wire          = require '../../main'

GET = 0x10



class Ambientlight

  address: 0x23

  constructor: (@address) ->
    @wire = new Wire @address;

  read: (callback) ->
    setTimeout =>
      @_read GET, 2, (err, buffer) ->
        if not err
          data = ((buffer[0] <<8) + buffer[1]) / 1.2;
          callback null, data
        else
          callback err, null
    , 200

  _read: (cmd, length, callback) ->
    @wire.readBytes cmd, length, callback

sensor = new Ambientlight(0x23)
sensor.read (err, data) ->
  console.log data

