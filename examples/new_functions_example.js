const i2c = require('../');

const ADDR = 0x42; // Replace with your device's address
const wire = new i2c(ADDR, { device: '/dev/i2c-1' });

function handleError(err) {
  if (err) {
    console.error('An error occurred:', err);
    wire.close();
    process.exit(1);
  }
}

function runExamples() {
  console.log('\n--- Interacting with device at address', ADDR, '---');

  // 1. writeQuick
  wire.writeQuick(1, (err) => {
    handleError(err);
    console.log('1. Wrote quick with bit 1');

    // 2. writeByte / readByte
    wire.writeByte(0x01, (err) => {
      handleError(err);
      console.log('2. Wrote byte 0x01');
      wire.readByte((err, byte) => {
        handleError(err);
        console.log('   Read byte:', byte);

        // 3. writeWord / readWord
        wire.writeWord(0x02, 0xABCD, (err) => {
          handleError(err);
          console.log('3. Wrote word 0xABCD to cmd 0x02');
          wire.readWord(0x02, (err, word) => {
            handleError(err);
            console.log('   Read word:', word.toString(16));

            // 4. writeByteData / readByteData
            wire.writeByteData(0x04, 0xEF, (err) => {
              handleError(err);
              console.log('4. Wrote byte 0xEF to cmd 0x04');
              wire.readByteData(0x04, (err, readByte) => {
                handleError(err);
                console.log('   Read byte from cmd 0x04:', readByte.toString(16));

                // 5. writeBlockData / readBlockData
                const block = Buffer.from([0x11, 0x22, 0x33]);
                wire.writeBlockData(0x05, block, (err) => {
                  handleError(err);
                  console.log('5. Wrote block to cmd 0x05:', block);
                  wire.readBlockData(0x05, (err, readBlock) => {
                    handleError(err);
                    console.log('   Read block from cmd 0x05:', readBlock);

                    // 6. readProcessCall
                    wire.readProcessCall(0x06, 0x9876, (err, result) => {
                      handleError(err);
                      console.log('6. Read process call result:', result.toString(16));

                      console.log('\n--- Finished ---');
                      wire.close();
                    });
                  });
                });
              });
            });
          });
        });
      });
    });
  });
}

console.log('Scanning for I2C devices...');
wire.scan((err, devices) => {
  handleError(err);
  console.log('Found devices:', devices);
  if (devices.includes(ADDR)) {
    runExamples();
  } else {
    console.log(`Device at address ${ADDR} not found.`);
    wire.close();
  }
});
