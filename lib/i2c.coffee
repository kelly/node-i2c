_            = require 'underscore'
wire         = require '../build/Release/i2c'
EventEmitter = require('events').EventEmitter

class i2c extends EventEmitter

  constructor: (device, @options = {}) ->    
    _.defaults @options,
      debug: false

    wire.open device, (err) -> 
      if err then throw err 

    if @options.debug 
      require('repl').start(
        prompt: "i2c > "
      ).context.wire = @
      process.stdin.emit 'data', 1 # trigger repl

    process.on 'exit', => @close()

  write: (addr, bytes, callback) ->
    wire.write addr, bytes, callback

  scan: (callback) ->
    wire.scan (err, data) ->
      callback err, _.filter data, (num) -> return num >= 0

  read: (addr, len, callback) ->
    wire.read addr, len, callback

  close: ->
    wire.close()

  stream: (addr, cmd, len = 1, delay = 100) ->
    wire.stream addr, cmd, len, delay, (err, data) =>
      if err
        @emit 'error', err
      else 
        @emit 'data', 
          data       : data
          address    : addr
          timestamp  : Date.now()
    @

module.exports = i2c