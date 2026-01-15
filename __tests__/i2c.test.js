const i2c = require('../main');
const wire = require('../build/Release/i2c');

jest.mock('../build/Release/i2c', () => ({
  open: jest.fn(),
  close: jest.fn(),
  scan: jest.fn(() => [0x18, 0x19, -1]),
  setAddress: jest.fn(),
  read: jest.fn((len) => Buffer.from(new Array(len).fill(0))),
  readByte: jest.fn(() => 0),
  readBlock: jest.fn((cmd, len) => Buffer.from(new Array(len).fill(cmd))),
  write: jest.fn(),
  writeByte: jest.fn(),
  writeBlock: jest.fn(),
}));

describe('i2c', () => {
  let bus;

  beforeEach(() => {
    bus = new i2c(0x18, { device: '/dev/i2c-1' });
  });

  afterEach(() => {
    bus.close();
    jest.clearAllMocks();
  });

  it('should open a connection', () => {
    bus = new i2c(0x18, { device: '/dev/i2c-1' });
    expect(wire.open).toHaveBeenCalledWith('/dev/i2c-1');
    expect(wire.setAddress).toHaveBeenCalledWith(0x18);
  });

  it('should scan for devices', async () => {
    const addresses = await bus.scan();
    expect(wire.scan).toHaveBeenCalled();
    expect(addresses).toEqual([0x18, 0x19]);
  });

  it('should close a connection', async () => {
    await bus.close();
    expect(wire.close).toHaveBeenCalled();
  });

  it('should read a buffer', async () => {
    const buffer = await bus.read(2);
    expect(wire.read).toHaveBeenCalledWith(2);
    expect(buffer).toEqual(Buffer.from([0, 0]));
  });

  it('should read a byte', async () => {
    const byte = await bus.readByte();
    expect(wire.readByte).toHaveBeenCalled();
    expect(byte).toBe(0);
  });

  it('should read a block of bytes', async () => {
    const buffer = await bus.readBytes(0x01, 2);
    expect(wire.readBlock).toHaveBeenCalledWith(0x01, 2);
    expect(buffer).toEqual(Buffer.from([0x01, 0x01]));
  });

  it('should write a buffer', async () => {
    const buffer = Buffer.from([0x01, 0x02]);
    await bus.write(buffer);
    expect(wire.write).toHaveBeenCalledWith(buffer);
  });

  it('should write a byte', async () => {
    await bus.writeByte(0x01);
    expect(wire.writeByte).toHaveBeenCalledWith(0x01);
  });

  it('should write a block of bytes', async () => {
    const buffer = Buffer.from([0x01, 0x02]);
    await bus.writeBytes(0x03, buffer);
    expect(wire.writeBlock).toHaveBeenCalledWith(0x03, buffer);
  });

  it('should stream data', (done) => {
    bus.on('data', (data) => {
      expect(data.data).toEqual(Buffer.from([0x01, 0x01]));
      bus.stopStream();
      done();
    });
    bus.stream(0x01, 2, 10);
  });

  it('should handle errors', async () => {
    const error = new Error('test error');
    wire.read.mockImplementation(() => { throw error; });
    await expect(bus.read(2)).rejects.toThrow(error);
  });
});
