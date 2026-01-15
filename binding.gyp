{
  "targets": [
    {
      "target_name": "i2c",
      "sources": [ "src/i2c.cc" ],
      "include_dirs" : [ 
          "<!(node -e \"require('nan')\")",
          "<!(node -e \"console.log(require('node-addon-api').include.replace(/\\\"/g, ''))\")"
      ],
      "cflags": [
        "-fno-exceptions"
      ],
      "cflags_cc": [
        "-fno-exceptions"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ]  
}
