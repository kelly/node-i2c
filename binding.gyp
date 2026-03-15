{
  "targets": [
    {
      "target_name": "i2c",
      "sources": [ "src/i2c.cc" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ]
    }
  ]
}
