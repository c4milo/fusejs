var Fuse = require('./fuse').Fuse
var f = new Fuse(); 

f.mount({
	mountpoint: '/tmp/cosa',
	filesystem: function() {}, 
	options: process.argv
});
