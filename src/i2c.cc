#include <node.h>
#include <v8.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include "smbus.h"
#include <unistd.h>

using namespace v8;
int fd;

void setAddress(int8_t addr) {
  int result = ioctl(fd, I2C_SLAVE, addr);
  if (result == -1) {
    ThrowException(
      Exception::TypeError(String::New("Failed to set address"))
    );
  }
}
void writeByte(int8_t byte) {
  int result = i2c_smbus_write_byte(fd, byte);
  if (result == -1) {
    ThrowException(
      Exception::TypeError(String::New("Cannot write to I2C"))
    );  
  }
}
int8_t readByte() {
  int result = i2c_smbus_read_byte(fd);
  if (result == -1) {
    ThrowException(
      Exception::TypeError(String::New("Cannot read byte"))
    );  
  }
  return result;
}
Handle<Value> Scan(const Arguments& args) {
  HandleScope scope;

  int i, res;
  Local<Array> results(Array::New(128));

  for (i = 0; i < 128; i++) {
    ioctl(fd, I2C_SLAVE, i);
    if ((i >= 0x30 && i <= 0x37) || (i >= 0x50 && i <= 0x5F)) {
      res = i2c_smbus_read_byte(fd);
    } else { 
      res = i2c_smbus_write_quick(fd, I2C_SMBUS_WRITE);
    }
    if (res >= 0) {
      res = i;
    }
    results->Set(i, Integer::New(res));
  }
  return scope.Close(results);
}

Handle<Value> Close(const Arguments& args) {
  HandleScope scope;

  return scope.Close(Undefined());
}

Handle<Value> Open(const Arguments& args) {
  HandleScope scope;

  String::Utf8Value device(args[0]);

  fd = open(*device, O_RDWR);
  if (fd == -1) {
    return ThrowException(
      Exception::TypeError(String::New("Failed to open I2C device"))
    );
  }
  return scope.Close(Undefined());
}

Handle<Value> Read(const Arguments& args) {
  HandleScope scope;

  int i;
  int8_t addr = args[0]->Int32Value();
  int len = args[1]->Int32Value();
  Local<Array> results(Array::New(len));

  setAddress(addr);

  for(i = 0; i < len; i++) {
    int8_t byte = readByte();
    results->Set(i, Integer::New(byte));
  }

  return scope.Close(results);
}

Handle<Value> Write(const Arguments& args) {
  HandleScope scope;

  int i;
  int len = args.Length();
  int8_t addr = args[0]->Int32Value();

  setAddress(addr);

  for (i = 1; i < len; i++) {
    int8_t byte = args[i]->Int32Value();
    writeByte(byte);
  }
  return scope.Close(Undefined());
}

Handle<Value> Stream(const Arguments& args) {
  HandleScope scope;

  int i;
  uint8_t addr   = args[0]->Int32Value();
  int32_t len    = args[1]->Int32Value();
  int32_t delay  = args[2]->Int32Value();

  if (!args[3]->IsFunction()) {
    return ThrowException(Exception::TypeError(
      String::New("Fourth argument must be a callback function")));
  }

  Local<Function> callback = Local<Function>::Cast(args[3]);
  Local<Value> results[len];

    while(1) {
      setAddress(addr);

      for(i = 0; i < len; i++) {
        int res = readByte();
        results[i] = Local<Value>::New(Integer::New(res));
      }

      callback->Call(Context::GetCurrent()->Global(), 1, results);
      usleep(delay);
    }

   return Undefined();
}

void Init(Handle<Object> target) {
  target->Set(String::NewSymbol("scan"),
    FunctionTemplate::New(Scan)->GetFunction());

  target->Set(String::NewSymbol("open"),
    FunctionTemplate::New(Open)->GetFunction());

  target->Set(String::NewSymbol("write"),
      FunctionTemplate::New(Write)->GetFunction());

  target->Set(String::NewSymbol("read"),
    FunctionTemplate::New(Read)->GetFunction());

  target->Set(String::NewSymbol("stream"),
    FunctionTemplate::New(Stream)->GetFunction());
}

NODE_MODULE(i2c, Init)