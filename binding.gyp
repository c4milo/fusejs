{
  "targets": [
        {
          "target_name": "fusejs",
          "sources": [ "src/bindings.cc", 
                        "src/file_info.cc", 
                        "src/filesystem.cc", 
                        "src/node_fuse.cc", 
                        "src/reply.cc", 
                        'src/forget_data.cc'                                                
                        ],
          "include_dirs": [
            './include',
            '<!@(pkg-config fuse --cflags-only-I | sed s/-I//g)',
            "<!(node -e \"require('nan')\")"
          ],
          "cflags_cc": [ 
            "-std=c++11",
            "-Wall", "-Warray-bounds", 
            "-fpermissive"
          ],
          "xcode_settings": {
             "OTHER_CPLUSPLUSFLAGS": ["-stdlib=libc++", "-std=c++11", "-mmacosx-version-min=10.7" ]
           },
          "defines": [
          '_FILE_OFFSET_BITS=64', 'FUSE_USE_VERSION=30'
          ],
          "link_settings": {
            "libraries": [
              '<!@(pkg-config --libs fuse)'
            ]
          }  
        }
      ]
}

