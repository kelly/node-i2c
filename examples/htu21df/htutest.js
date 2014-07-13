var i2c_htu21d = require('./htu21df');

var htu21df = new i2c_htu21d({device: '/dev/i2c-1'});

htu21df.readTemperature(function (temp) {
    console.log('Temperature, C:', temp);

    htu21df.readHumidity(function (humidity) {
        console.log('Humidity, RH %:', humidity);
    });
});
