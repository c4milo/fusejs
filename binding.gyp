{
  "targets": [
        {
          "target_name": "fusejs",
          "sources": [ "src/bindings.cc","src/bindings.h", "src/file_info.cc","src/file_info.h", "src/filesystem.cc","src/filesystem.h", "src/node_fuse.cc","src/node_fuse.h", "src/reply.cc","src/reply.h"],
          "include_dirs": [
            './ck/',
            '<!@(pkg-config fuse --cflags-only-I | sed s/-I//g)'
          ],
          'cflags_cc': ['-Wall', '-g', '-Warray-bounds', '-fpermissive'],
          "defines": [
          '_FILE_OFFSET_BITS=64', 'FUSE_USE_VERSION=27', 'restrict='
          ],
          "link_settings": {
            "libraries": [
              '<!@(pkg-config --libs fuse ck)'
            ]
          }  
        }
      ]
}
