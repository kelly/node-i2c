#include <napi.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <vector>
#include "i2c-dev.h"

int fd;
int8_t addr;

void setAddress(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "addr must be an int").ThrowAsJavaScriptException();
    return;
  }
  addr = info[0].As<Napi::Number>().Int32Value();
  if (ioctl(fd, I2C_SLAVE_FORCE, addr) == -1) {
    Napi::TypeError::New(env, "Failed to set address").ThrowAsJavaScriptException();
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
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "device must be a string").ThrowAsJavaScriptException();
    return;
  }
  std::string device = info[0].As<Napi::String>();
  fd = open(device.c_str(), O_RDWR);
  if (fd == -1) {
    Napi::Error::New(env, "Failed to open I2C device").ThrowAsJavaScriptException();
  }
}

Napi::Value Read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "len must be an int").ThrowAsJavaScriptException();
    return env.Null();
  }
  int len = info[0].As<Napi::Number>().Int32Value();
  std::vector<char> buf(len);
  if (read(fd, buf.data(), len) != len) {
    Napi::Error::New(env, "Cannot read from device").ThrowAsJavaScriptException();
    return env.Null();
  }
  return Napi::Buffer<char>::Copy(env, buf.data(), len);
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
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "cmd and len must be ints").ThrowAsJavaScriptException();
    return env.Null();
  }
  int8_t cmd = info[0].As<Napi::Number>().Int32Value();
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
  if (info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::TypeError::New(env, "buffer must be a Buffer").ThrowAsJavaScriptException();
    return;
  }
  Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
  if (write(fd, buffer.Data(), buffer.Length()) != (ssize_t)buffer.Length()) {
    Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
  }
}

void WriteByte(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "byte must be an int").ThrowAsJavaScriptException();
    return;
  }
  int8_t byte = info[0].As<Napi::Number>().Int32Value();
  if (i2c_smbus_write_byte(fd, byte) == -1) {
    Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
  }
}

void WriteBlock(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsBuffer()) {
    Napi::TypeError::New(env, "cmd must be an int and buffer must be a Buffer").ThrowAsJavaScriptException();
    return;
  }
  int8_t cmd = info[0].As<Napi::Number>().Int32Value();
  Napi::Buffer<uint8_t> buffer = info[1].As<Napi::Buffer<uint8_t>>();
  if (i2c_smbus_write_i2c_block_data(fd, cmd, buffer.Length(), buffer.Data()) == -1) {
    Napi::Error::New(env, "Cannot write to device").ThrowAsJavaScriptException();
  }
}

void WriteWord(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "cmd and word must be ints").ThrowAsJavaScriptException();
    return;
  }
  int8_t cmd = info[0].As<Napi::Number>().Int32Value();
  int16_t word = info[1].As<Napi::Number>().Int32Value();
  if (i2c_smbus_write_word_data(fd, cmd, word) == -1) {
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
  exports.Set(Napi::String::New(env, "writeWord"), Napi::Function::New(env, WriteWord));
  exports.Set(Napi::String::New(env, "read"), Napi::Function::New(env, Read));
  exports.Set(Napi::String::New(env, "readByte"), Napi::Function::New(env, ReadByte));
  exports.Set(Napi::String::New(env, "readBlock"), Napi::Function::New(env, ReadBlock));
  return exports;
}

NODE_API_MODULE(i2c, Init)
