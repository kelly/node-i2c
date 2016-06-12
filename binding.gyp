{
  "targets": [
    {
      "target_name": "i2c",
      "sources": [ "src/i2c.cc" ],
      "include_dirs" : [ 
          "<!(node -e \"require('nan')\")" 
      ]
    }
  ]  
}
