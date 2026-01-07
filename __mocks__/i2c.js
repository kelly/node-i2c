module.exports = {
  scan: (cb) => cb(null, []),
  setAddress: () => {},
  open: (device, cb) => cb(null),
  close: () => {},
  write: (buf, cb) => cb(null),
  writeByte: (byte, cb) => cb(null),
  writeBlock: (cmd, buf, cb) => cb(null),
  read: (len, cb) => cb(null, Buffer.from([])),
  readByte: (cb) => cb(null, 0),
  readBlock: (cmd, len, delay, cb) => cb(null, Buffer.from([])),
};
