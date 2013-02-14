_            = require 'underscore'
wire         = require '../build/Release/i2c'
EventEmitter = require('events').EventEmitter

class i2c extends EventEmitter

  constructor: (device, @options = {}) ->    
    _.defaults @options,
      debug: true

    wire.open(device)

    process.on 'exit', -> wire.close()

  write: (addr, bytes) ->
    wire.write(addr, bytes);

  scan: ->
    result = _.filter wire.scan(), (num) -> return num >= 0

  read: (addr, cmd, len = 1) ->
    wire.read addr, cmd, len

  stream: (addr, cmd, len = 1, delay = 100) ->
    wire.stream addr, len, delay, (data) =>
      @emit 'stream:read', 
        data       : data
        address    : addr
        timestamp  : Date.now()

module.exports = i2c