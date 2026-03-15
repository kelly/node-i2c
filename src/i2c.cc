#include <node_api.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "i2c-dev.h"

#define MAX_READ_LEN 1024
#define I2C_ADDR_COUNT 128

typedef struct {
  int fd;
  int8_t addr;
  bool is_open;
} I2CDevice;

static void device_destructor(napi_env env, void* data, void* hint) {
  I2CDevice* device = (I2CDevice*)data;
  if (device->is_open) {
    close(device->fd);
  }
  free(device);
}

// --- Helper: unwrap device from `this` ---

static I2CDevice* unwrap_device(napi_env env, napi_callback_info info) {
  napi_value jsthis;
  napi_get_cb_info(env, info, NULL, NULL, &jsthis, NULL);
  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);
  return device;
}

// --- Helper: check device is open, throw if not ---

static bool require_open(napi_env env, I2CDevice* device) {
  if (!device->is_open) {
    napi_throw_error(env, NULL, "Device is not open");
    return false;
  }
  return true;
}

// --- Constructor ---

static napi_value New(napi_env env, napi_callback_info info) {
  napi_value jsthis;
  napi_get_cb_info(env, info, NULL, NULL, &jsthis, NULL);

  I2CDevice* device = (I2CDevice*)malloc(sizeof(I2CDevice));
  if (!device) {
    napi_throw_error(env, NULL, "Failed to allocate I2CDevice");
    return NULL;
  }
  device->fd = -1;
  device->addr = 0;
  device->is_open = false;

  napi_wrap(env, jsthis, device, device_destructor, NULL, NULL);
  return jsthis;
}

// --- Synchronous: SetAddress ---

static napi_value SetAddress(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t addr;
  napi_get_value_int32(env, argv[0], &addr);

  device->addr = (int8_t)addr;
  int result = ioctl(device->fd, I2C_SLAVE_FORCE, device->addr);
  if (result == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Failed to set address 0x%02x: %s", addr, strerror(errno));
    napi_throw_error(env, NULL, msg);
  }

  return NULL;
}

// --- Synchronous: Close ---

static napi_value Close(napi_env env, napi_callback_info info) {
  I2CDevice* device = unwrap_device(env, info);
  if (device->is_open) {
    close(device->fd);
    device->fd = -1;
    device->is_open = false;
  }
  return NULL;
}

// ============================================================
// Async worker infrastructure
//
// Every async operation holds a napi_ref to the JS `this` object
// to prevent garbage collection of the I2CDevice while the
// worker thread is accessing it.
//
// Before queuing work, we snapshot the fd from the device so
// the worker thread uses a stable copy. If the device is closed
// concurrently, the worst case is an EBADF error (not a use of
// a recycled fd), because close() sets device->fd = -1 and the
// kernel won't reuse an fd until close() returns.
// ============================================================

// --- Open ---

typedef struct {
  I2CDevice* device;
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  char device_path[256];
  int result_fd;
  int err_no;
} OpenWorkData;

static void open_execute(napi_env env, void* data) {
  OpenWorkData* d = (OpenWorkData*)data;
  d->result_fd = open(d->device_path, O_RDWR);
  if (d->result_fd == -1) {
    d->err_no = errno;
  }
}

static void open_complete(napi_env env, napi_status status, void* data) {
  OpenWorkData* d = (OpenWorkData*)data;

  napi_value callback, global, argv[1];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->result_fd == -1) {
    char msg[512];
    snprintf(msg, sizeof(msg), "Failed to open I2C device %s: %s", d->device_path, strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else {
    d->device->fd = d->result_fd;
    d->device->is_open = true;
    napi_get_null(env, &argv[0]);
  }

  napi_call_function(env, global, callback, 1, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value Open(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  OpenWorkData* d = (OpenWorkData*)malloc(sizeof(OpenWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for open operation");
    return NULL;
  }
  d->device = device;
  d->err_no = 0;

  size_t path_len;
  napi_get_value_string_utf8(env, argv[0], d->device_path, sizeof(d->device_path), &path_len);

  napi_create_reference(env, argv[1], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:open", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, open_execute, open_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- Scan ---

typedef struct {
  I2CDevice* device;
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;  // snapshot of device->fd at queue time
  int8_t original_addr;
  int results[I2C_ADDR_COUNT];
} ScanWorkData;

static void scan_execute(napi_env env, void* data) {
  ScanWorkData* d = (ScanWorkData*)data;
  int fd = d->fd;

  for (int i = 0; i < I2C_ADDR_COUNT; i++) {
    if (ioctl(fd, I2C_SLAVE_FORCE, i) < 0) {
      d->results[i] = -1;
      continue;
    }

    int res;
    if ((i >= 0x30 && i <= 0x37) || (i >= 0x50 && i <= 0x5F)) {
      res = i2c_smbus_read_byte(fd);
    } else {
      res = i2c_smbus_write_quick(fd, I2C_SMBUS_WRITE);
    }
    d->results[i] = (res >= 0) ? i : -1;
  }

  // Restore original address
  ioctl(fd, I2C_SLAVE_FORCE, d->original_addr);
}

static void scan_complete(napi_env env, napi_status status, void* data) {
  ScanWorkData* d = (ScanWorkData*)data;

  napi_value callback, global, argv[2];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  napi_get_null(env, &argv[0]);

  napi_value results;
  napi_create_array_with_length(env, I2C_ADDR_COUNT, &results);
  for (int i = 0; i < I2C_ADDR_COUNT; i++) {
    napi_value val;
    napi_create_int32(env, d->results[i], &val);
    napi_set_element(env, results, i, val);
  }
  argv[1] = results;

  napi_call_function(env, global, callback, 2, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value Scan(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  ScanWorkData* d = (ScanWorkData*)malloc(sizeof(ScanWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for scan operation");
    return NULL;
  }
  d->device = device;
  d->fd = device->fd;
  d->original_addr = device->addr;

  napi_create_reference(env, argv[0], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:scan", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, scan_execute, scan_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- Read ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;
  int len;
  uint8_t* buf;
  int bytes_read;
  int err_no;
} ReadWorkData;

static void read_execute(napi_env env, void* data) {
  ReadWorkData* d = (ReadWorkData*)data;
  d->bytes_read = read(d->fd, d->buf, d->len);
  if (d->bytes_read < 0) {
    d->err_no = errno;
  }
}

static void read_complete(napi_env env, napi_status status, void* data) {
  ReadWorkData* d = (ReadWorkData*)data;

  napi_value callback, global, argv[2];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->bytes_read < 0) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot read from device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
    napi_get_undefined(env, &argv[1]);
  } else {
    napi_get_null(env, &argv[0]);
    napi_value arr;
    napi_create_array_with_length(env, d->bytes_read, &arr);
    for (int i = 0; i < d->bytes_read; i++) {
      napi_value val;
      napi_create_int32(env, d->buf[i], &val);
      napi_set_element(env, arr, i, val);
    }
    argv[1] = arr;
  }

  napi_call_function(env, global, callback, 2, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d->buf);
  free(d);
}

static napi_value Read(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t len;
  napi_get_value_int32(env, argv[0], &len);

  if (len <= 0 || len > MAX_READ_LEN) {
    napi_throw_range_error(env, NULL, "read length must be between 1 and 1024");
    return NULL;
  }

  ReadWorkData* d = (ReadWorkData*)malloc(sizeof(ReadWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for read operation");
    return NULL;
  }
  d->fd = device->fd;
  d->len = len;
  d->buf = (uint8_t*)malloc(len);
  if (!d->buf) {
    free(d);
    napi_throw_error(env, NULL, "Failed to allocate read buffer");
    return NULL;
  }
  d->bytes_read = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[1], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:read", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, read_execute, read_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- ReadByte ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;
  int32_t result;
  int err_no;
} ReadByteWorkData;

static void read_byte_execute(napi_env env, void* data) {
  ReadByteWorkData* d = (ReadByteWorkData*)data;
  d->result = i2c_smbus_read_byte(d->fd);
  if (d->result == -1) {
    d->err_no = errno;
  }
}

static void read_byte_complete(napi_env env, napi_status status, void* data) {
  ReadByteWorkData* d = (ReadByteWorkData*)data;

  napi_value callback, global, argv[2];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->result == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot read byte from device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
    napi_get_undefined(env, &argv[1]);
  } else {
    napi_get_null(env, &argv[0]);
    napi_create_int32(env, d->result, &argv[1]);
  }

  napi_call_function(env, global, callback, 2, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value ReadByte(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  ReadByteWorkData* d = (ReadByteWorkData*)malloc(sizeof(ReadByteWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for readByte operation");
    return NULL;
  }
  d->fd = device->fd;
  d->result = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[0], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:readByte", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, read_byte_execute, read_byte_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- ReadBlock (single shot, no loop) ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;
  uint8_t cmd;
  uint8_t len;
  uint8_t buf[I2C_SMBUS_BLOCK_MAX];
  int32_t bytes_read;
  int err_no;
} ReadBlockWorkData;

static void read_block_execute(napi_env env, void* data) {
  ReadBlockWorkData* d = (ReadBlockWorkData*)data;
  d->bytes_read = i2c_smbus_read_i2c_block_data(d->fd, d->cmd, d->len, d->buf);
  if (d->bytes_read == -1) {
    d->err_no = errno;
  }
}

static void read_block_complete(napi_env env, napi_status status, void* data) {
  ReadBlockWorkData* d = (ReadBlockWorkData*)data;

  napi_value callback, global, argv[2];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->bytes_read == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot read block from device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
    napi_get_undefined(env, &argv[1]);
  } else {
    napi_get_null(env, &argv[0]);
    void* buf_data;
    napi_value buffer;
    napi_create_buffer_copy(env, d->bytes_read, d->buf, &buf_data, &buffer);
    argv[1] = buffer;
  }

  napi_call_function(env, global, callback, 2, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value ReadBlock(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value argv[3], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t cmd, len;
  napi_get_value_int32(env, argv[0], &cmd);
  napi_get_value_int32(env, argv[1], &len);

  if (len <= 0 || len > I2C_SMBUS_BLOCK_MAX) {
    napi_throw_range_error(env, NULL, "readBlock length must be between 1 and 32");
    return NULL;
  }

  ReadBlockWorkData* d = (ReadBlockWorkData*)malloc(sizeof(ReadBlockWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for readBlock operation");
    return NULL;
  }
  d->fd = device->fd;
  d->cmd = (uint8_t)cmd;
  d->len = (uint8_t)len;
  d->bytes_read = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[2], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:readBlock", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, read_block_execute, read_block_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- Write ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  napi_ref buffer_ref;
  int fd;
  uint8_t* buf;
  size_t len;
  ssize_t bytes_written;
  int err_no;
} WriteWorkData;

static void write_execute(napi_env env, void* data) {
  WriteWorkData* d = (WriteWorkData*)data;
  d->bytes_written = write(d->fd, d->buf, d->len);
  if (d->bytes_written < 0) {
    d->err_no = errno;
  }
}

static void write_complete(napi_env env, napi_status status, void* data) {
  WriteWorkData* d = (WriteWorkData*)data;

  napi_value callback, global, argv[1];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->bytes_written < 0) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot write to device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else if ((size_t)d->bytes_written != d->len) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Partial write to device: wrote %zd of %zu bytes",
             d->bytes_written, d->len);
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else {
    napi_get_null(env, &argv[0]);
  }

  napi_call_function(env, global, callback, 1, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_reference(env, d->buffer_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value Write(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  WriteWorkData* d = (WriteWorkData*)malloc(sizeof(WriteWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for write operation");
    return NULL;
  }
  d->fd = device->fd;

  napi_get_buffer_info(env, argv[0], (void**)&d->buf, &d->len);
  // Hold a reference to the buffer to prevent GC during async work
  napi_create_reference(env, argv[0], 1, &d->buffer_ref);

  d->bytes_written = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[1], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:write", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, write_execute, write_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- WriteByte ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;
  uint8_t byte;
  int32_t result;
  int err_no;
} WriteByteWorkData;

static void write_byte_execute(napi_env env, void* data) {
  WriteByteWorkData* d = (WriteByteWorkData*)data;
  d->result = i2c_smbus_write_byte(d->fd, d->byte);
  if (d->result == -1) {
    d->err_no = errno;
  }
}

static void write_byte_complete(napi_env env, napi_status status, void* data) {
  WriteByteWorkData* d = (WriteByteWorkData*)data;

  napi_value callback, global, argv[1];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->result == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot write byte to device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else {
    napi_get_null(env, &argv[0]);
  }

  napi_call_function(env, global, callback, 1, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value WriteByte(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t byte;
  napi_get_value_int32(env, argv[0], &byte);

  WriteByteWorkData* d = (WriteByteWorkData*)malloc(sizeof(WriteByteWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for writeByte operation");
    return NULL;
  }
  d->fd = device->fd;
  d->byte = (uint8_t)byte;
  d->result = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[1], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:writeByte", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, write_byte_execute, write_byte_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- WriteBlock ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  napi_ref buffer_ref;
  int fd;
  uint8_t cmd;
  uint8_t* buf;
  size_t len;
  int32_t result;
  int err_no;
} WriteBlockWorkData;

static void write_block_execute(napi_env env, void* data) {
  WriteBlockWorkData* d = (WriteBlockWorkData*)data;
  d->result = i2c_smbus_write_i2c_block_data(d->fd, d->cmd, d->len, d->buf);
  if (d->result == -1) {
    d->err_no = errno;
  }
}

static void write_block_complete(napi_env env, napi_status status, void* data) {
  WriteBlockWorkData* d = (WriteBlockWorkData*)data;

  napi_value callback, global, argv[1];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->result == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot write block to device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else {
    napi_get_null(env, &argv[0]);
  }

  napi_call_function(env, global, callback, 1, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_reference(env, d->buffer_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value WriteBlock(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value argv[3], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t cmd;
  napi_get_value_int32(env, argv[0], &cmd);

  WriteBlockWorkData* d = (WriteBlockWorkData*)malloc(sizeof(WriteBlockWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for writeBlock operation");
    return NULL;
  }
  d->fd = device->fd;
  d->cmd = (uint8_t)cmd;

  napi_get_buffer_info(env, argv[1], (void**)&d->buf, &d->len);
  napi_create_reference(env, argv[1], 1, &d->buffer_ref);

  d->result = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[2], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:writeBlock", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, write_block_execute, write_block_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// --- WriteWord ---

typedef struct {
  napi_async_work work;
  napi_ref callback_ref;
  napi_ref this_ref;
  int fd;
  uint8_t cmd;
  uint16_t word;
  int32_t result;
  int err_no;
} WriteWordWorkData;

static void write_word_execute(napi_env env, void* data) {
  WriteWordWorkData* d = (WriteWordWorkData*)data;
  d->result = i2c_smbus_write_word_data(d->fd, d->cmd, d->word);
  if (d->result == -1) {
    d->err_no = errno;
  }
}

static void write_word_complete(napi_env env, napi_status status, void* data) {
  WriteWordWorkData* d = (WriteWordWorkData*)data;

  napi_value callback, global, argv[1];
  napi_get_reference_value(env, d->callback_ref, &callback);
  napi_get_global(env, &global);

  if (d->result == -1) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Cannot write word to device: %s", strerror(d->err_no));
    napi_value err_msg;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &err_msg);
    napi_create_error(env, NULL, err_msg, &argv[0]);
  } else {
    napi_get_null(env, &argv[0]);
  }

  napi_call_function(env, global, callback, 1, argv, NULL);
  napi_delete_reference(env, d->callback_ref);
  napi_delete_reference(env, d->this_ref);
  napi_delete_async_work(env, d->work);
  free(d);
}

static napi_value WriteWord(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value argv[3], jsthis;
  napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL);

  I2CDevice* device;
  napi_unwrap(env, jsthis, (void**)&device);

  if (!require_open(env, device)) return NULL;

  int32_t cmd, word;
  napi_get_value_int32(env, argv[0], &cmd);
  napi_get_value_int32(env, argv[1], &word);

  WriteWordWorkData* d = (WriteWordWorkData*)malloc(sizeof(WriteWordWorkData));
  if (!d) {
    napi_throw_error(env, NULL, "Failed to allocate memory for writeWord operation");
    return NULL;
  }
  d->fd = device->fd;
  d->cmd = (uint8_t)cmd;
  d->word = (uint16_t)word;
  d->result = 0;
  d->err_no = 0;

  napi_create_reference(env, argv[2], 1, &d->callback_ref);
  napi_create_reference(env, jsthis, 1, &d->this_ref);

  napi_value resource_name;
  napi_create_string_utf8(env, "i2c:writeWord", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(env, NULL, resource_name, write_word_execute, write_word_complete, d, &d->work);
  napi_queue_async_work(env, d->work);

  return NULL;
}

// ============================================================
// Module initialization
// ============================================================

static napi_value Init(napi_env env, napi_value exports) {
  napi_value cons;

  napi_property_descriptor methods[] = {
    { "open",       NULL, Open,       NULL, NULL, NULL, napi_default, NULL },
    { "close",      NULL, Close,      NULL, NULL, NULL, napi_default, NULL },
    { "setAddress", NULL, SetAddress, NULL, NULL, NULL, napi_default, NULL },
    { "scan",       NULL, Scan,       NULL, NULL, NULL, napi_default, NULL },
    { "read",       NULL, Read,       NULL, NULL, NULL, napi_default, NULL },
    { "readByte",   NULL, ReadByte,   NULL, NULL, NULL, napi_default, NULL },
    { "readBlock",  NULL, ReadBlock,  NULL, NULL, NULL, napi_default, NULL },
    { "write",      NULL, Write,      NULL, NULL, NULL, napi_default, NULL },
    { "writeByte",  NULL, WriteByte,  NULL, NULL, NULL, napi_default, NULL },
    { "writeBlock", NULL, WriteBlock, NULL, NULL, NULL, napi_default, NULL },
    { "writeWord",  NULL, WriteWord,  NULL, NULL, NULL, napi_default, NULL },
  };

  napi_define_class(env, "I2CDevice", NAPI_AUTO_LENGTH, New, NULL,
                    sizeof(methods) / sizeof(methods[0]), methods, &cons);

  napi_set_named_property(env, exports, "I2CDevice", cons);

  return exports;
}

NAPI_MODULE(i2c, Init)
