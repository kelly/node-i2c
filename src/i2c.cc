#include <napi.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include "i2c-dev.h"

int fd;
int8_t addr;

void setAddress(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int8_t addr = (int8_t)info[0].As<Napi::Number>().Int32Value();
  int result = ioctl(fd, I2C_SLAVE_FORCE, addr);
  if (result == -1) {
    Napi::Error::New(env, "Failed to set address").ThrowAsJavaScriptException();
  }
}

Napi::Value Scan(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  Napi::Array results = Napi::Array::New(env, 128);
  for (int i = 0; i < 128; i++) {
    ioctl(fd, I2C_SLAVE_FORCE, i);
    int res;
    if ((i >= 0x30 && i <= 0x37) || (i >= 0x50 && i <= 0x5F)) {
      res = i2c_smbus_read_byte(fd);
    } else {
      res = i2c_smbus_write_quick(fd, I2C_SMBUS_WRITE);
    }
    if (res >= 0) {
      res = i;
    }
    results[i] = Napi::Number::New(env, res);
  }

  ioctl(fd, I2C_SLAVE_FORCE, addr);

  return results;
}

void Close(const Napi::CallbackInfo& info) {
  if (fd > 0) {
    close(fd);
  }
}

void Open(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string device = info[0].As<Napi::String>().Utf8Value();
  fd = open(device.c_str(), O_RDWR);
  if (fd == -1) {
    Napi::Error::New(env, "Failed to open I2C device").ThrowAsJavaScriptException();
  }
}

Napi::Value Read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int len = info[0].As<Napi::Number>().Int32Value();
  char* buf = new char[len];
  if (read(fd, buf, len) != len) {
    delete[] buf;
    Napi::Error::New(env, "Cannot read from device").ThrowAsJavaScriptException();
    return env.Null();
  }
  Napi::Buffer<char> buffer = Napi::Buffer<char>::Copy(env, buf, len);
  delete[] buf;
  return buffer;
}

Napi::Value ReadByte(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int32_t res = i2c_smbus_read_byte(fd);
  if (res == -1) {
    Napi::Error::New(env, "Cannot read device").ThrowAsJavaScriptException();
    return env.Null();
  }
  return Napi::Number::New(env, res);
}

Napi::Value ReadBlock(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int8_t cmd = (int8_t)info[0].As<Napi::Number>().Int32Value();
    int32_t len = info[1].As<Napi::Number>().Int32Value();
    std::vector<uint8_t> data(len);

    if (i2c_smbus_read_i2c_block_data(fd, cmd, len, data.data()) != len) {
        Napi::Error::New(env, "Error reading length of bytes").ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::Buffer<uint8_t>::Copy(env, data.data(), len);
}


void Write(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
  if (write(fd, buffer.Data(), buffer.Length()) != (ssize_t)buffer.Length()) {
    Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
  }
}

void WriteByte(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int8_t byte = (int8_t)info[0].As<Napi::Number>().Int32Value();
  if (i2c_smbus_write_byte(fd, byte) == -1) {
    Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
  }
}

void WriteBlock(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int8_t cmd = (int8_t)info[0].As<Napi::Number>().Int32Value();
    Napi::Buffer<unsigned char> buffer = info[1].As<Napi::Buffer<unsigned char>>();

    if (i2c_smbus_write_i2c_block_data(fd, cmd, buffer.Length(), buffer.Data()) == -1) {
        Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
    }
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "setAddress"), Napi::Function::New(env, setAddress));
  exports.Set(Napi::String::New(env, "scan"), Napi::Function::New(env, Scan));
  exports.Set(Napi::String::New(env, "open"), Napi::Function::New(env, Open));
  exports.Set(Napi::String::New(env, "close"), Napi::Function::New(env, Close));
  exports.Set(Napi::String::New(env, "write"), Napi::Function::New(env, Write));
  exports.Set(Napi::String::New(env, "writeByte"), Napi::Function::New(env, WriteByte));
  exports.Set(Napi::String::New(env, "writeBlock"), Napi::Function::New(env, WriteBlock));
  exports.Set(Napi::String::New(env, "read"), Napi::Function::New(env, Read));
  exports.Set(Napi::String::New(env, "readByte"), Napi::Function::New(env, ReadByte));
  exports.Set(Napi::String::New(env, "readBlock"), Napi::Function::New(env, ReadBlock));
  return exports;
}

NODE_API_MODULE(i2c, Init)
