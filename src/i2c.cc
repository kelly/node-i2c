#include <node.h>
#include <v8.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "i2c-dev.h"
#include <unistd.h>

using namespace v8;
int fd;

void setAddress(int8_t addr) {
  ioctl(fd, I2C_SLAVE_FORCE, addr);
}
int8_t writeByte(int8_t byte) {
  return i2c_smbus_write_byte(fd, byte);
}
int8_t readByte() {
  return i2c_smbus_read_byte(fd);
}
Handle<Value> Scan(const Arguments& args) {
  HandleScope scope;

  int i, res;
  Local<Function> callback = Local<Function>::Cast(args[0]);
  Local<Array> results(Array::New(128));
  Local<Value> err = Local<Value>::New(Null());

  for (i = 0; i < 128; i++) {
    setAddress(i);
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
  const unsigned argc = 2;
  Local<Value> argv[argc] = { err, results };
  callback->Call(Context::GetCurrent()->Global(), argc, argv);

  return scope.Close(results);
}

Handle<Value> Close(const Arguments& args) {
  HandleScope scope;

  if (fd > 0) {
    close(fd);
  }
  return scope.Close(Undefined());
}

Handle<Value> Open(const Arguments& args) {
  HandleScope scope;

  String::Utf8Value device(args[0]);
  Local<Value> err = Local<Value>::New(Null());

  fd = open(*device, O_RDWR);
  if (fd == -1) {
    err = Exception::Error(String::New("Failed to open I2C device"));
  }

  if (args[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    Local<Value> argv[argc] = { err };

    callback->Call(Context::GetCurrent()->Global(), argc, argv);
  }

  return scope.Close(Undefined());
}

Handle<Value> Read(const Arguments& args) {
  HandleScope scope;

  int i;
  int8_t addr = args[0]->Int32Value();
  int len     = args[1]->Int32Value();
  Local<Function> callback = Local<Function>::Cast(args[2]);
  Local<Value> err = Local<Value>::New(Null());
  Local<Array> results(Array::New(len));

  setAddress(addr);

  for(i = 0; i < len; i++) {
    int8_t res = readByte();
    if (res == -1) { 
      err = Exception::Error(String::New("Cannot read device"));
    } else {
      results->Set(i, Number::New(res));
    }
  }
  const unsigned argc = 2;
  Local<Value> argv[argc] = { err, results };
  callback->Call(Context::GetCurrent()->Global(), argc, argv);

  return scope.Close(results);
}

Handle<Value> Write(const Arguments& args) {
  HandleScope scope;

  int8_t addr = args[0]->Int32Value();
  int8_t res;

  Local<Value> err = Local<Value>::New(Null());

  setAddress(addr);

  if (args[1]->IsArray()) {
    Local<Array> bytes = Array::Cast(*args[1]);
    int len = bytes->Length();
    int i;
    for (i = 0; i < len; i++) {
      int8_t byte = bytes->Get(i)->Int32Value();
      res = writeByte(byte);
    }
  } else {
    int8_t byte = args[1]->Int32Value();
    res = writeByte(byte);
  }
  if (res == -1) {
    err = Exception::Error(String::New("Cannot write to device"));
  }

  if (args[2]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[2]);
    Local<Value> argv[argc] = { err };

    callback->Call(Context::GetCurrent()->Global(), argc, argv);
  }

  return scope.Close(Undefined());
}

Handle<Value> Stream(const Arguments& args) {
  HandleScope scope;

  int i;
  int8_t addr   = args[0]->Int32Value();
  int8_t cmd    = args[1]->Int32Value();
  int32_t len   = args[2]->Int32Value();
  int32_t delay = args[3]->Int32Value();
  int8_t res;

  Local<Function> callback = Local<Function>::Cast(args[4]);
  Local<Array> results(Array::New(len));
  Local<Value> err = Local<Value>::New(Null());

  setAddress(addr);

  while(fd) {
    if(writeByte(cmd) == 0) { 
      for(i = 0; i < len; i++) {
        res = readByte();
        if(res == -1) { 
          err = Exception::Error(String::New("Cannot read device"));
        } else {
          results->Set(i, Integer::New(res));
        }
      }
      const unsigned argc = 2;
      Local<Value> argv[argc] = { err, results };
      callback->Call(Context::GetCurrent()->Global(), argc, argv);
      usleep(delay);
    } else {
        err = Exception::Error(String::New("Cannot write to device"));
        const unsigned argc = 1;
        Local<Value> argv[argc] = { err };
        callback->Call(Context::GetCurrent()->Global(), argc, argv);
    }
  }

   return Undefined();
}

void Init(Handle<Object> target) {
  target->Set(String::NewSymbol("scan"),
    FunctionTemplate::New(Scan)->GetFunction());

  target->Set(String::NewSymbol("open"),
    FunctionTemplate::New(Open)->GetFunction());

  target->Set(String::NewSymbol("close"),
    FunctionTemplate::New(Close)->GetFunction());

  target->Set(String::NewSymbol("write"),
      FunctionTemplate::New(Write)->GetFunction());

  target->Set(String::NewSymbol("read"),
    FunctionTemplate::New(Read)->GetFunction());

  target->Set(String::NewSymbol("stream"),
    FunctionTemplate::New(Stream)->GetFunction());
}

NODE_MODULE(i2c, Init)