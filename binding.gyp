


{
    "targets": [{
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
        "conditions": [
            ['OS!="win"', {
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
            }],
            ['OS=="win"', {
                "variables": {
                    'dokan__install_dir%': '$(DokanLibrary1)/include/fuse'
                },
                "include_dirs": [
                    "<(dokan__install_dir)",
                    "$(INCLUDE)"
                ],
                "link_settings": {
                    "libraries": [
                        "<(dokan__library)"
                    ]
                },
                "conditions": [
                    ['target_arch=="x64"', {
                        "variables": { 'dokan__library%': '$(DokanLibrary1_LibraryPath_x64)/dokanfuse1' }
                    }, {
                        "variables": { 'dokan__library%': '$(DokanLibrary1_LibraryPath_x86)/dokanfuse1' }
                    }]
                ]
            }]
        ],
        "configurations": {
            "Debug": {
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 2
                    }
                }
            },
            "Release": {
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 2
                    }
                }
            }
        }
    }]
}