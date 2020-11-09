#include <node.h>
#include <node_buffer.h>
#include <nan.h>
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
  Nan::HandleScope scope;

  int result = ioctl(fd, I2C_SLAVE_FORCE, addr);
  if (result == -1) {
    isolate->ThrowException(
      Exception::TypeError(Nan::New("Failed to set address").ToLocalChecked())
    );
    return;
  }
}

void SetAddress(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("addr must be an int");
    return;
  }
  addr = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  setAddress(addr);
}

void Scan(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  int i, res;
  Local<Function> callback = Local<Function>::Cast(info[0]);
  Local<Array> results = Nan::New<Array>(128);
  Local<Value> err = Nan::New<Value>(Nan::Null());

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
    results->Set(
      Nan::GetCurrentContext(),
      i,
      Nan::New<Integer>(res)
    );
  }

  setAddress(addr);

  const unsigned argc = 2;
  Local<Value> argv[argc] = { err, results };

  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);

  info.GetReturnValue().Set(results);
}

void Close(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  if (fd > 0) {
    close(fd);
  }
}

void Open(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  Nan::Utf8String device(info[0]);
  Local<Value> err = Nan::New<Value>(Nan::Null());

  fd = open(*device, O_RDWR);
  if (fd == -1) {
    err = Nan::Error(Nan::New("Failed to open I2C device").ToLocalChecked());
  }

  if (info[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(info[1]);
    Local<Value> argv[argc] = { err };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void Read(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  int len = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  Local<Array> data = Nan::New<Array>();

  char* buf = new char[len];
  Local<Value> err = Nan::New<Value>(Nan::Null());

  if (read(fd, buf, len) != len) {
    err = Nan::Error(Nan::New("Cannot read from device").ToLocalChecked());
  } else {
    for (int i = 0; i < len; ++i) {
      data->Set(Nan::GetCurrentContext(), i, Nan::New<Integer>(buf[i]));
    }
  }
  delete[] buf;

  if (info[1]->IsFunction()) {
    const unsigned argc = 2;
    Local<Function> callback = Local<Function>::Cast(info[1]);
    Local<Value> argv[argc] = { err, data };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void ReadByte(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;
  
  Local<Value> data; 
  Local<Value> err = Nan::New<Value>(Nan::Null());

  int32_t res = i2c_smbus_read_byte(fd);

  if (res == -1) { 
    err = Nan::Error(Nan::New("Cannot read device").ToLocalChecked());
  } else {
    data = Nan::New<Integer>(res);
  }

  if (info[0]->IsFunction()) {
    const unsigned argc = 2;
    Local<Function> callback = Local<Function>::Cast(info[0]);
    Local<Value> argv[argc] = { err, data };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }

  info.GetReturnValue().Set(data);
}

void ReadBlock(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  int8_t cmd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int32_t len = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  uint8_t data[len]; 
  Local<Value> err = Nan::New<Value>(Nan::Null());
  // Local<Object> buffer = node::Buffer::New(len);
  //new version for Nan
  Local<Object> buffer = Nan::NewBuffer(len).ToLocalChecked();  //todo  - some error checking here as the devs intended?


  while (fd > 0) {
    if (i2c_smbus_read_i2c_block_data(fd, cmd, len, data) != len) {
      err = Nan::Error(Nan::New("Error reading length of bytes").ToLocalChecked());
    }

    memcpy(node::Buffer::Data(buffer), data, len);

    if (info[3]->IsFunction()) {
      const unsigned argc = 2;
      Local<Function> callback = Local<Function>::Cast(info[3]);
      Local<Value> argv[argc] = { err, buffer };
      Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
    }
 
    if (info[2]->IsNumber()) {
      int32_t delay = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
      usleep(delay * 1000);
    } else {
      break;
    }
  }

  info.GetReturnValue().Set(buffer);
}

void Write(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

//  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  auto bufferObj = info[0]->ToObject(Nan::GetCurrentContext()).ToLocalChecked();

  int   len = node::Buffer::Length(bufferObj);
  char* data = node::Buffer::Data(bufferObj);

  Local<Value> err = Nan::New<Value>(Nan::Null());

  if (write(fd, (unsigned char*) data, len) != len) {
    err = Nan::Error(Nan::New("Cannot write to device").ToLocalChecked());
  }

  if (info[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(info[1]);
    Local<Value> argv[argc] = { err };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void WriteByte(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  int8_t byte = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Local<Value> err = Nan::New<Value>(Nan::Null());

  if (i2c_smbus_write_byte(fd, byte) == -1) {
    err = Nan::Error(Nan::New("Cannot write to device").ToLocalChecked());
  }

  if (info[1]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(info[1]);
    Local<Value> argv[argc] = { err };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void WriteBlock(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;

  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  auto bufferObj = info[1]->ToObject(context).ToLocalChecked();

  int8_t cmd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int   len = node::Buffer::Length(bufferObj);
  char* data = node::Buffer::Data(bufferObj);

  Local<Value> err = Nan::New<Value>(Nan::Null());

  if (i2c_smbus_write_i2c_block_data(fd, cmd, len, (unsigned char*) data) == -1) {
    err = Nan::Error(Nan::New("Cannot write to device").ToLocalChecked());
  }

  if (info[2]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(info[2]);
    Local<Value> argv[argc] = { err };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void WriteWord(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::HandleScope scope;
  
  int8_t cmd = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int16_t word = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  Local<Value> err = Nan::New<Value>(Nan::Null());
  
  if (i2c_smbus_write_word_data(fd, cmd, word) == -1) {
    err = Nan::Error(Nan::New("Cannot write to device").ToLocalChecked());
  }

  if (info[2]->IsFunction()) {
    const unsigned argc = 1;
    Local<Function> callback = Local<Function>::Cast(info[2]);
    Local<Value> argv[argc] = { err };
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
  }
}

void Init(Local<Object> exports) {

  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("setAddress").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(SetAddress)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("scan").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(Scan)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("open").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(Open)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("close").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(Close)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("write").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(Write)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("writeByte").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(WriteByte)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("writeBlock").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(WriteBlock)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("read").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(Read)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("readByte").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(ReadByte)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );
  exports->Set(
    Nan::GetCurrentContext(),
    Nan::New("readBlock").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(ReadBlock)->GetFunction(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Function>())
  );

}

NODE_MODULE(i2c, Init)
