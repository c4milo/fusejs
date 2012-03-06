var fuse = require('../fuse');

describe('fuse_common.h', function() {
    before(function(done) {
        done();
    });

    it('fuse_version', function(done) {
        fuse.fuse_version.should.be.a('number');
        done();
    });

    it('fuse_mount');
    it('fuse_unmount');
    it('fuse_daemonize');
    it('fuse_set_signal_handlers');
    it('fuse_remove_signal_handlers');
});

describe('fuse_lowlevel.h', function() {
    describe('fuse_reply_*', function() {
        it('fuse_reply_err');
        it('fuse_reply_none');
        it('fuse_reply_entry');
        it('fuse_reply_create');
        it('fuse_reply_attr');
        it('fuse_reply_readlink');
        it('fuse_reply_open');
        it('fuse_reply_write');
        it('fuse_reply_buf');
        it('fuse_reply_iov');
        it('fuse_reply_statfs');
        it('fuse_reply_xattr');
        it('fuse_reply_lock');
        it('fuse_reply_bmap');
        it('fuse_reply_ioctl_retry');
        it('fuse_reply_ioctl');
        it('fuse_reply_poll');
    });

    describe('fuse_req_*', function() {
        it('fuse_req_userdata');
        it('fuse_req_ctx');
        it('fuse_req_interrupt_func');
        it('fuse_req_interrupted');
    });

    describe('fuse_session_*', function() {
        it('fuse_session_new');
        it('fuse_session_add_chan');
        it('fuse_session_remove_chan');
        it('fuse_session_next_chan');
        it('fuse_session_process');
        it('fuse_session_destroy');
        it('fuse_session_exit');
        it('fuse_session_reset');
        it('fuse_session_exited');
        it('fuse_session_loop');
    });

    describe('fuse_chan_*', function() {
        it('fuse_chan_fd');
        it('fuse_chan_bufsize');
        it('fuse_chan_data');
        it('fuse_chan_session');
        it('fuse_chan_recv');
        it('fuse_chan_send');
        it('fuse_chan_destroy');
    });

    describe('misc', function() {
        it('fuse_add_direntry');
        it('fuse_lowlevel_notify_poll');
        it('fuse_lowlevel_new');
    });
});
