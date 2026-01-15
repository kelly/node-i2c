const path = require('path');
const { expect } = require('chai');

// Resolve the path to the native addon
const i2cAddonPath = require.resolve('../build/Release/i2c.node');

// Load the mock
const mockI2C = require('./mock-i2c');

// Inject the mock into the require cache
require.cache[i2cAddonPath] = { exports: mockI2C };

// Now, when we require the i2c module, it will use the mocked native addon
const I2C = require('../lib/i2c');

describe('I2C', () => {
  let i2c;

  beforeEach(() => {
    // Re-assign the variable to handle the case where require is cached
    const I2CWithMock = require('../lib/i2c');
    i2c = new I2CWithMock(0x42, { device: '/dev/i2c-test' });
  });

  after(() => {
    // Clean up the require cache
    delete require.cache[i2cAddonPath];
  });

  it('should scan for devices', (done) => {
    i2c.scan((err, data) => {
      expect(err).to.be.null;
      expect(data).to.deep.equal([0x01, 0x02, 0x03]);
      done();
    });
  });

  it('should write a buffer', (done) => {
    const buffer = Buffer.from([0x01, 0x02]);
    i2c.write(buffer, (err) => {
      expect(err).to.be.null;
      done();
    });
  });

  it('should write a byte', (done) => {
    i2c.writeByte(0x01, (err) => {
      expect(err).to.be.null;
      done();
    });
  });

  it('should write bytes', (done) => {
    const buffer = Buffer.from([0x01, 0x02]);
    i2c.writeBytes(0x01, buffer, (err) => {
      expect(err).to.be.null;
      done();
    });
  });

  it('should read a buffer', (done) => {
    i2c.read(2, (err, data) => {
      expect(err).to.be.null;
      expect(data).to.be.instanceOf(Buffer);
      expect(data.length).to.equal(2);
      done();
    });
  });

  it('should read a byte', (done) => {
    i2c.readByte((err, data) => {
      expect(err).to.be.null;
      expect(data).to.equal(0x04);
      done();
    });
  });

  it('should read bytes', (done) => {
    i2c.readBytes(0x01, 2, (err, data) => {
      expect(err).to.be.null;
      expect(data).to.be.instanceOf(Buffer);
      expect(data.length).to.equal(2);
      done();
    });
  });

  it('should stream data', (done) => {
    i2c.on('data', (data) => {
      expect(data).to.have.property('address', 0x42);
      expect(data).to.have.property('data');
      expect(data.data.length).to.equal(2);
      expect(data).to.have.property('cmd', 0x01);
      expect(data).to.have.property('length', 2);
      expect(data).to.have.property('timestamp');
      done();
    });
    i2c.stream(0x01, 2, 10);
  });
});
