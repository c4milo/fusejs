{
  "targets": [
        {
          "target_name": "fusejs",
          "sources": [ "src/bindings.cc", 
                        "src/file_info.cc", 
                        "src/filesystem.cc", 
                        "src/node_fuse.cc", 
                        "src/reply.cc",                                                 
                        ],
          "include_dirs": [
            '<!@(pkg-config fuse --cflags-only-I | sed s/-I//g)',
            "<!(node -e \"require('nan')\")"
          ],
          'cflags_cc': ['-Wall', '-g', '-Warray-bounds', '-fpermissive'],
          "defines": [
          '_FILE_OFFSET_BITS=64', 'FUSE_USE_VERSION=27', 'restrict='
          ],
          "link_settings": {
            "libraries": [
              '<!@(pkg-config --libs fuse)'
            ]
          }  
        }
      ]
}

