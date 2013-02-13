var i2c = require('./build/Release/i2c');

random = function() {
  return parseInt(Math.random() * 255, 8);
};
i2c.open("/dev/i2c-0");
setInterval(function() {
  // i2c.writeByte(0x18, 0x63);
  // i2c.writeByte(0x18, random());
  // i2c.writeByte(0x18, random());
  // i2c.writeByte(0x18, random());

  // i2c.write(0x18, 0x63, random());
  // console.log(i2c.read(0x18, 1));

  i2c.write(0x18, 0x63, random(), random(), random());
  // i2c.write(0x18, 0x5a);
  // console.log(i2c.read(0x18, 1));
  // i2c.write(0x18, 0x63, random(), random(), random());
}, 100);
  
// i2c.write(0x18, 0x67);
// i2c.stream(0x18, 3, 100, function(data) {
//   console.log(data);
// });
// console.log(i2c.scan());
// console.log(i2c.read(0x5a));