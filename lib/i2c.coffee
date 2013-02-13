wire = require '../build/Release/i2c'
_    = require 'underscore'
# repl = require 'repl'

class i2c

  constructor: (device) ->
    _.defaults @options,
      delay: 100

    wire.open(device)

    process.on 'exit', ->
      wire.close()

  write: (addr, bytes) ->
    wire.write(addr, bytes);

  scan: ->
    result = _.filter wire.scan(), (num) -> return num >= 0

  read: (addr, len) ->
    wire.read(addr, len)

  stream: (addr, len, callback) ->
    wire.stream(addr, len, @options.delay, callback)

module.exports = i2c