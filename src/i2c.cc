#include <node.h>
#include <v8.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include "smbus.h"

using namespace v8;

Handle<Value> Write(const Arguments& args) {
  HandleScope scope;

  int fd;
  int result;
  int8_t addr = args[0]->Int32Value();
  int8_t data = args[1]->Int32Value();

  fd = open("/dev/i2c-0", O_RDWR);
  ioctl(fd, I2C_SLAVE, addr);

  if (fd == -1) {
    return ThrowException(
      Exception::TypeError(String::New("Failed to open I2C device"))
    );
  }
  result = i2c_smbus_write_byte(fd, data);
  if (result == -1) {
    return ThrowException(
      Exception::TypeError(String::New("Cannot write to I2C"))
    );  
  }
  return scope.Close(Undefined());
}

void Init(Handle<Object> target) {
  target->Set(String::NewSymbol("write"),
      FunctionTemplate::New(Write)->GetFunction());
}

NODE_MODULE(i2c, Init)