var fuse = require('../fuse');

describe('Fuse', function() {
    before(function(done) {
        done();
    });

    it('should expose fuse version', function(done) {
        fuse.fuse_version.should.be.a('number');
        done();
    });

    it('should mount a filesystem', function(done) {
        done();
    });
});
