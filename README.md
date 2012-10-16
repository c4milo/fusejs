## FUSEJS - Low level bindings for Fuse
Fusejs are a set of bindings for [Fuse](http://fuse.sourceforge.net/) low level API. Even though most of the Fuse functions are already binded, this module has not been used in production. Use it at your own risk. 


## How it works
Fuse low level API is inherently asynchronous, meaning that it won't block the nodejs main thread nor is needed [libuv](https://github.com/joyent/libuv) to make use of its thread pool. FuseJS workflow looks like:

```                          
                        node example/hello_main.js /tmp/hello_fs
                                         ↕ 
                                Google V8 / FuseJS
                                         ↕
	ls -lah /tmp/hello_fs             libfuse
         	  ↕                          ↕
       		glibc                      glibc
Userspace     ↕                          ↕
---------------------------------------------         
Kernel        ↕                          ↕	
			  ↕                          ↕
             VFS ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔  FUSE
             
             							...
                                        Ext4
                                        NFS
                                        ZFS
```

## Installation
### OSX
In order to use FuseJS you need to install any of the Fuse implementations for OSX. OSXFuse is the one that has been used throughout the FuseJS development. Go to http://osxfuse.github.com/ and follow the instructions to get it installed. In addition, FuseJS toolchain uses `pkg-config`, you need to have it installed in your system as well, in order to compile FuseJS. It usually should come by default in your operating system, if not, then use your package manager to install it.

* download https://github.com/downloads/osxfuse/osxfuse/OSXFUSE-2.5.2.dmg
* sudo port install pkg-config (OSX)
* ```npm install fusejs``` 


### Linux
I haven't tested Linux yet. 



## API Documentation
All the API Documentation can be found at https://github.com/c4milo/fusejs/blob/master/fuse.js. You can also take a look at the examples in https://github.com/c4milo/fusejs/tree/master/examples

## License
(The MIT License)

Copyright 2012 Camilo Aguilar. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
