_            = require 'underscore'
wire         = require '../build/Release/i2c'
EventEmitter = require('events').EventEmitter
tick         = setImmediate || process.nextTick

class i2c extends EventEmitter

  history: []

  constructor: (@address, @options = {}) ->
    _.defaults @options,
      debug: false
      device: "/dev/i2c-1"

    if @options.debug 
      require('repl').start(
        prompt: "i2c > "
      ).context.wire = @
      process.stdin.emit 'data', '' # trigger repl

    process.on 'exit', => @close()

    @on 'data', (data) => 
      @history.push data

    @on 'error', (err) ->
      console.log "Error: #{err}"

    @open @options.device, (err) =>
      unless err then @setAddress @address

  scan: (callback) ->
    wire.scan (err, data) ->
      tick ->
        callback err, _.filter data, (num) -> return num >= 0

  setAddress: (address) ->
    wire.setAddress address
    @address = address

  open: (device, callback) ->
    wire.open device, (err) ->
      tick ->
        callback err

  close: ->
    wire.close()

  write: (buf, callback) ->
    @setAddress @address
    unless Buffer.isBuffer(buf) then buf = new Buffer(buf)
    wire.write buf, (err) ->
      tick ->
        callback err

  writeByte: (byte, callback) ->
    @setAddress @address
    wire.writeByte byte, (err) ->
      tick ->
        callback err

  writeBytes: (cmd, buf, callback) ->
    @setAddress @address
    unless Buffer.isBuffer(buf) then buf = new Buffer(buf)
    wire.writeBlock cmd, buf, (err) ->
      tick ->
        callback err

  read: (len, callback) ->
    @setAddress @address
    wire.read len, (err, data) ->
      tick ->
        callback err, data

  readByte: (callback) ->
    @setAddress @address
    wire.readByte (err, data) ->
      tick ->
        callback err, data

  readBytes: (cmd, len, callback) ->
    @setAddress @address
    wire.readBlock cmd, len, null, (err, actualBuffer) ->
      tick ->
        callback err, actualBuffer

  stream: (cmd, len, delay = 100) ->
    @setAddress @address
    wire.readBlock cmd, len, delay, (err, data) =>
      if err 
        @emit 'error', err
      else 
        @emit 'data', 
          address    : @address
          data       : data
          cmd        : cmd
          length     : len
          timestamp  : Date.now()

module.exports = i2c
