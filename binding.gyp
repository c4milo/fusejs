{
  "targets": [
        {
          "target_name": "fusejs",
          "sources": [ "src/bindings.cc","src/file_info.cc","src/filesystem.cc","src/node_fuse.cc","src/reply.cc"],
          "include_dirs": [
             '<!@(pkg-config fuse --cflags-only-I | sed s/-I//g)',
          ],
          "link_settings": {
            "libraries": [
              '<!@(pkg-config --libs-only-l fuse)',
              "-L/usr/local/lib"
            ]
          }
        }
      ]
}
