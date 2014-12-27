// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include <stdlib.h>
#include "reply.h"
#include "file_info.h"
#include "node_buffer.h"
#define MIN(x, y) ((x) < (y) ? (x) : (y))

namespace NodeFuse {
    Persistent<FunctionTemplate> Reply::constructor_template;

    void Reply::Initialize() {
        Local<FunctionTemplate> t = NanNew<FunctionTemplate>();

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "entry", Reply::Entry);
        NODE_SET_PROTOTYPE_METHOD(t, "attr", Reply::Attributes);
        NODE_SET_PROTOTYPE_METHOD(t, "readlink", Reply::ReadLink);
        NODE_SET_PROTOTYPE_METHOD(t, "err", Reply::Error);
        NODE_SET_PROTOTYPE_METHOD(t, "open", Reply::Open);
        NODE_SET_PROTOTYPE_METHOD(t, "buffer", Reply::Buffer);
        NODE_SET_PROTOTYPE_METHOD(t, "write", Reply::Write);
        NODE_SET_PROTOTYPE_METHOD(t, "statfs", Reply::StatFs);
        NODE_SET_PROTOTYPE_METHOD(t, "create", Reply::Create);
        NODE_SET_PROTOTYPE_METHOD(t, "xattr", Reply::XAttributes);
        NODE_SET_PROTOTYPE_METHOD(t, "addDirEntry", Reply::AddDirEntry);

        NanAssignPersistent(constructor_template,t);
        // constructor_template->SetClassName(NanNew<String>("Reply"));
    }

    Reply::Reply() : ObjectWrap() {
        dentry_acc_size = 0;
        dentry_cur_length = 0;
        dentry_size = 0;
        dentry_offset = 0;
        dentry_buffer = NULL;
    }

    Reply::~Reply() {
        if (dentry_buffer != NULL) {
            free(dentry_buffer);
        }
    }

    NAN_METHOD(Reply::Entry) {
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            NanThrowTypeError( "You must specify an object as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(arg, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            NanReturnUndefined();
        }

        ret = fuse_reply_entry(reply->request, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Attributes){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            NanThrowTypeError( "You must specify an object as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        struct stat statbuff;

        ret = ObjectToStat(arg, &statbuff);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized stat object: ", "Unable to reply the operation");
            NanReturnUndefined();
        }

        double timeout = 0;
        if (argslen == 2) {
            if (!args[1]->IsNumber()) {
                FUSEJS_THROW_EXCEPTION("Invalid timeout, ", "it should be the number of seconds in which the attributes are considered valid.");
                NanReturnUndefined();
            }

            timeout = args[1]->NumberValue();
        }

        ret = fuse_reply_attr(reply->request, &statbuff, timeout);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::ReadLink){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsString()) {
            NanThrowTypeError( "You must specify a string as first argument");
            NanReturnUndefined();
        }

        String::Utf8Value link(arg->ToString());

        int ret = -1;
        ret = fuse_reply_readlink(reply->request, (const char*) *link);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Error){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            NanThrowTypeError( "You must specify a number as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        ret = fuse_reply_err(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Open){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Object> fiobj = args[0]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            NanThrowTypeError( "You must specify a FileInfo object as first argument");
            NanReturnUndefined();
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        int ret = -1;
        ret = fuse_reply_open(reply->request, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Buffer){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError( "You must specify 2 arguments to invoke this function");
            NanReturnUndefined();
        }

        if (!Buffer::HasInstance(args[0])) {
            NanThrowTypeError( "You must specify a Buffer object as first argument");
            NanReturnUndefined();

        }
        if (!args[1]->IsNumber()) {
            NanThrowTypeError( "You must specify the size of the buffer");
            NanReturnUndefined();

        }

        int ret = -1;
        size_t size = args[1]->IntegerValue();

        if (reply->dentry_buffer == NULL){
            Local<Object> buffer = args[0]->ToObject();
            const char* data = Buffer::Data(buffer);
            ret = fuse_reply_buf( reply->request, data, size);
        }else{

            if (reply->dentry_offset < reply->dentry_acc_size){
                ret = fuse_reply_buf(reply->request, reply->dentry_buffer + reply->dentry_offset, MIN(reply->dentry_acc_size - reply->dentry_offset,size) );
            }else{
                ret = fuse_reply_buf(reply->request, NULL, 0 );
            }

        }
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Write){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            NanThrowTypeError( "You must specify the number of bytes written as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        ret = fuse_reply_write(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::StatFs){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int ret = -1;
        struct statvfs buf;

        int argslen = args.Length();

        if (argslen == 0) {
            NanThrowTypeError( "You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            NanThrowTypeError( "You must specify a object as first argument");
            NanReturnUndefined();
        }

        ret = ObjectToStatVfs(arg, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized statvfs object: ", "Unable to reply the operation");
            NanReturnUndefined();
        }

        ret = fuse_reply_statfs(reply->request, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Create){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 2) {
            NanThrowTypeError( "You must specify at least two arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> params = args[0];
        if (!params->IsObject()) {
            NanThrowTypeError( "You must specify an object as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(params, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            NanReturnUndefined();
        }

        Local<Object> fiobj = args[1]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            NanThrowTypeError( "You must specify a FileInfo object as second argument");
            NanReturnUndefined();
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        ret = fuse_reply_create(reply->request, &entry, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::XAttributes){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError("You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            NanThrowTypeError("You must specify a number as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        ret = fuse_reply_xattr(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::Lock){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            NanThrowTypeError("You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            NanThrowTypeError("You must specify a Lock object as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        struct flock lock;
        ret = ObjectToFlock(arg, &lock);

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized lock object: ", "Unable to reply the operation");
            NanReturnUndefined();
        }

        ret = fuse_reply_lock(reply->request, &lock);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::BMap){
        NanScope();

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            NanThrowTypeError("You must specify arguments to invoke this function");
            NanReturnUndefined();
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            NanThrowTypeError("You must specify a number as first argument");
            NanReturnUndefined();
        }

        int ret = -1;
        ret = fuse_reply_bmap(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            NanReturnUndefined();
        }

        NanReturnUndefined();
    }

    NAN_METHOD(Reply::AddDirEntry) {
        NanScope();
        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 4) {
            NanThrowTypeError("You must specify four arguments to invoke this function");
            NanReturnUndefined();
        }

        if (!args[0]->IsString()) {
            NanThrowTypeError("You must specify an entry name String as first argument");
            NanReturnUndefined();
        }

        if (!args[1]->IsNumber()) {
            NanThrowTypeError("You must specify the requested size number as second argument");
            NanReturnUndefined();
        }

        if (!args[2]->IsObject()) {
            NanThrowTypeError("You must specify stat Object as third argument");
            NanReturnUndefined();
        }

        if (!args[3]->IsNumber()) {
            NanThrowTypeError("You must specify a offset number as fourth argument");
            NanReturnUndefined();
        }

        String::Utf8Value name(args[0]->ToString());
        size_t requestedSize = args[1]->IntegerValue();
        off_t offset = args[3]->IntegerValue();
        reply-> dentry_offset = offset;

        char* buffer = reply->dentry_buffer;

        struct stat statbuff;
        ObjectToStat(args[2]->ToObject(), &statbuff);


        size_t acc_size = reply->dentry_acc_size;

        size_t len = fuse_add_direntry(reply->request, NULL, 0, *name, &statbuff, 0);
        buffer = (char * )realloc(buffer, acc_size + len);
        reply->dentry_buffer = buffer;
        size_t len2 = fuse_add_direntry(reply->request, (char*) (buffer + acc_size),
         requestedSize - acc_size,
         *name, &statbuff, acc_size + len);
        
        // fprintf(stderr, "Current length! -> %d\n", (int)reply->dentry_cur_length);

        // fprintf(stderr, "Entry name -> %s\n", (const char*) *name);
        // fprintf(stderr, "Space needed for the entry -> %d or %d \n", (int) len, (int)len2);
        // fprintf(stderr, "Requested size -> %d\n", (int) requestedSize);
        // fprintf(stderr, "Remaning buffer -> %d\n", (int)(requestedSize - acc_size));
        // fprintf(stderr, "Offset -> %lld\n\n", offset );        

        reply->dentry_acc_size += len;
        reply->dentry_cur_length++;

        NanReturnValue(NanNew<Integer>(len2));
    }
} //ends namespace NodeFuse
