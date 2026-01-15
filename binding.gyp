{
  "targets": [
    {
      "target_name": "i2c",
      "sources": [ "src/i2c.cc" ],
      'defines': [ 'NAPI_CPP_EXCEPTIONS' ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions', '-std=c++11' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "include_dirs" : [
          "<!(node -e \"require('node-addon-api').include\")"
      ]
    }
  ]  
}
