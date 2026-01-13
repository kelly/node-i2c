const chai = require('chai');
const expect = chai.expect;

describe('i2c', () => {
  it('should be requireable', () => {
    const i2c = require('../');
    expect(i2c).to.be.an('function');
  });
});
