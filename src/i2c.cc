#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "i2c-dev.h"

using namespace v8;
int fd;
int8_t addr;

void setAddress(int8_t addr) {
  Isolate* isolate = Isolate::GetCurrent();

  int result = ioctl(fd, I2C_SLAVE_FORCE, addr);
  if (result == -1) {
    isolate->ThrowException(
      Exception::TypeError(String::NewFromUtf8(isolate, "Failed to set address"))
    );
    return;
  }
}

void SetAddress(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  addr = args[0]->Int32Value();
  setAddress(addr);
}

void Scan(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  int i, res;
  Local<Function> callback = Local<Function>::Cast(args[0]);
  Local<Array> results = Array::New(isolate, 128);
  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

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
    results->Set(i, Integer::New(isolate, res));
  }

  setAddress(addr);

  const unsigned argc = 2;
  Local<Value> argv[argc] = { err, results };
  callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);

  args.GetReturnValue().Set(results);
}

void Close(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (fd > 0) {
    close(fd);
  }
}

void Open(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  String::Utf8Value device(args[0]);

  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  fd = open(*device, O_RDWR);
  if (fd == -1) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Failed to open I2C device"));
  }

  if (args[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    Local<Value> argv[argc] = { err };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void Read(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  int len = args[0]->Int32Value();

  Local<Array> data = Array::New(isolate);

  char* buf = new char[len];
  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  if (read(fd, buf, len) != len) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot read from device"));
  } else {
    for (int i = 0; i < len; ++i) {
      data->Set(i, Integer::New(isolate, buf[i]));
    }
  }
  delete[] buf;

  if (args[1]->IsFunction()) {
    const unsigned argc = 2;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    Local<Value> argv[argc] = { err, data };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void ReadByte(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  
  Local<Value> data; 
  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  int32_t res = i2c_smbus_read_byte(fd);

  if (res == -1) { 
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot read device"));
  } else {
    data = Integer::New(isolate, res);
  }

  if (args[0]->IsFunction()) {
    const unsigned argc = 2;
    Local<Function> callback = Local<Function>::Cast(args[0]);
    Local<Value> argv[argc] = { err, data };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }

  args.GetReturnValue().Set(data);
}

void ReadBlock(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  int8_t cmd = args[0]->Int32Value();
  int32_t len = args[1]->Int32Value();
  uint8_t data[len]; 
  Local<Value> err = Local<Value>::New(isolate, Null(isolate));
  Local<Object> buffer = node::Buffer::New(len);

  while (fd > 0) {
    if (i2c_smbus_read_i2c_block_data(fd, cmd, len, data) != len) {
      err = Exception::Error(String::NewFromUtf8(isolate, "Error reading length of bytes"));
    }

    memcpy(node::Buffer::Data(buffer), data, len);

    if (args[3]->IsFunction()) {
      const unsigned argc = 2;
      Local<Function> callback = Local<Function>::Cast(args[3]);
      Local<Value> argv[argc] = { err, buffer };
      callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
    }
 
    if (args[2]->IsNumber()) {
      int32_t delay = args[2]->Int32Value();
      usleep(delay * 1000);
    } else {
      break;
    }
  }

  args.GetReturnValue().Set(buffer);
}

void Write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Value> buffer = args[0];

  int   len = node::Buffer::Length(buffer->ToObject());
  char* data = node::Buffer::Data(buffer->ToObject());

  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  if (write(fd, (unsigned char*) data, len) != len) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot write to device"));
  }

  if (args[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    Local<Value> argv[argc] = { err };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void WriteByte(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  int8_t byte = args[0]->Int32Value();
  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  if (i2c_smbus_write_byte(fd, byte) == -1) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot write to device"));
  }

  if (args[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    Local<Value> argv[argc] = { err };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void WriteBlock(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Value> buffer = args[1];

  int8_t cmd = args[0]->Int32Value();
  int   len = node::Buffer::Length(buffer->ToObject());
  char* data = node::Buffer::Data(buffer->ToObject());

  Local<Value> err = Local<Value>::New(isolate, Null(isolate));

  if (i2c_smbus_write_i2c_block_data(fd, cmd, len, (unsigned char*) data) == -1) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot write to device"));
  }

  if (args[2]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[2]);
    Local<Value> argv[argc] = { err };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void WriteWord(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  
  int8_t cmd = args[0]->Int32Value();
  int16_t word = args[1]->Int32Value();

  Local<Value> err = Local<Value>::New(isolate, Null(isolate));
  
  if (i2c_smbus_write_word_data(fd, cmd, word) == -1) {
    err = Exception::Error(String::NewFromUtf8(isolate, "Cannot write to device"));
  }

  if (args[2]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(args[2]);
    Local<Value> argv[argc] = { err };

    callback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
  }
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "setAddress", SetAddress);
  NODE_SET_METHOD(exports, "scan", Scan);
  NODE_SET_METHOD(exports, "open", Open);
  NODE_SET_METHOD(exports, "close", Close);
  NODE_SET_METHOD(exports, "write", Write);
  NODE_SET_METHOD(exports, "writeByte", WriteByte);
  NODE_SET_METHOD(exports, "writeBlock", WriteBlock);
  NODE_SET_METHOD(exports, "read", Read);
  NODE_SET_METHOD(exports, "readByte", ReadByte);
  NODE_SET_METHOD(exports, "readBlock", ReadBlock);
}

NODE_MODULE(i2c, Init)