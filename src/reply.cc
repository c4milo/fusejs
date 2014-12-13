// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include <stdlib.h>
#include "reply.h"
#include "file_info.h"
#include "node_buffer.h"
#define MIN(x, y) ((x) < (y) ? (x) : (y))

namespace NodeFuse {
    Persistent<FunctionTemplate> Reply::constructor_template;

    void Reply::Initialize() {
        Local<FunctionTemplate> t = FunctionTemplate::New();

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

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("Reply"));
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

    Handle<Value> Reply::Entry(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify an object as first argument")));
            return scope.Close( Null() ) ;
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(arg, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            return scope.Close( Null() ) ;
        }

        ret = fuse_reply_entry(reply->request, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close( Null() ) ;
        }

        return scope.Close( Undefined() ) ;
    }

    Handle<Value> Reply::Attributes(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close( Undefined() );
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify an object as first argument")));
            return scope.Close( Undefined() );
        }

        int ret = -1;
        struct stat statbuff;

        ret = ObjectToStat(arg, &statbuff);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized stat object: ", "Unable to reply the operation");
            return scope.Close(  Null());
        }

        double timeout = 0;
        if (argslen == 2) {
            if (!args[1]->IsNumber()) {
                FUSEJS_THROW_EXCEPTION("Invalid timeout, ", "it should be the number of seconds in which the attributes are considered valid.");
                return scope.Close(  Null());
            }

            timeout = args[1]->NumberValue();
        }

        ret = fuse_reply_attr(reply->request, &statbuff, timeout);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(  Null());
        }

        return scope.Close(  Undefined());
    }

    Handle<Value> Reply::ReadLink(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsString()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a string as first argument")));
            return scope.Close(Undefined());
        }

        String::Utf8Value link(arg->ToString());

        int ret = -1;
        ret = fuse_reply_readlink(reply->request, (const char*) *link);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close( Null() );
        }

        return scope.Close( Undefined() );
    }

    Handle<Value> Reply::Error(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a number as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        ret = fuse_reply_err(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(Null());
        }

        return scope.Close(Undefined());
    }

    Handle<Value> Reply::Open(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Object> fiobj = args[0]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a FileInfo object as first argument")));
            return scope.Close(Undefined());
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        int ret = -1;
        ret = fuse_reply_open(reply->request, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(Null());
        }

        return scope.Close(Undefined());
    }

    Handle<Value> Reply::Buffer(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify 2 arguments to invoke this function")));
            return scope.Close( Undefined());
        }

        if (!Buffer::HasInstance(args[0])) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a Buffer object as first argument")));
            return scope.Close( Undefined());

        }
        if (!args[1]->IsNumber()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify the size of the buffer")));
            return scope.Close( Undefined());

        }

        int ret = -1;
        size_t size = args[1]->IntegerValue();

        if (reply->dentry_buffer == NULL){
            printf("req after read %llu\n", reply->request ); 
            Local<Object> buffer = args[0]->ToObject();
            const char* data = Buffer::Data(buffer);
            // const char *str = "hello world";
            const char* persistent_data = (char*) malloc(size);
            memcpy( (void *) persistent_data, data, size);
            // printf("buffer length %d - %d\n",Buffer::Length(buffer), size);
            // ret = fuse_reply_buf(reply->request, persistent_data, size);
            ret = fuse_reply_buf( reply->request, persistent_data, size);
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
        printf("end of the reply buffer");

        return scope.Close(Undefined() );
    }

    Handle<Value> Reply::Write(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify the number of bytes written as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        ret = fuse_reply_write(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(Null());
        }

        return scope.Close(Undefined());
    }

    Handle<Value> Reply::StatFs(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int ret = -1;
        struct statvfs buf;

        int argslen = args.Length();

        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a object as first argument")));
            return scope.Close(Null());
        }

        ret = ObjectToStatVfs(arg, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized statvfs object: ", "Unable to reply the operation");
            return scope.Close(Null());
        }

        ret = fuse_reply_statfs(reply->request, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(Null());
        }

        return scope.Close(Undefined());
    }

    Handle<Value> Reply::Create(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 2) {
            ThrowException(Exception::TypeError(
                String::New("You must specify at least two arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> params = args[0];
        if (!params->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify an object as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(params, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            return scope.Close( Null() );
        }

        Local<Object> fiobj = args[1]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a FileInfo object as second argument")));
            return scope.Close( Null());
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        ret = fuse_reply_create(reply->request, &entry, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(  Null() );
        }

        return scope.Close(  Undefined() );
    }

    Handle<Value> Reply::XAttributes(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a number as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        ret = fuse_reply_xattr(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close( Null() );
        }

        return scope.Close( Undefined() );
    }

    Handle<Value> Reply::Lock(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a Lock object as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        struct flock lock;
        ret = ObjectToFlock(arg, &lock);

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized lock object: ", "Unable to reply the operation");
            return scope.Close(Null());
        }

        ret = fuse_reply_lock(reply->request, &lock);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close( Null() );
        }

        return scope.Close( Undefined());
    }

    Handle<Value> Reply::BMap(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            ThrowException(Exception::TypeError(
                String::New("You must specify arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a number as first argument")));
            return scope.Close(Undefined());
        }

        int ret = -1;
        ret = fuse_reply_bmap(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return scope.Close(  Null() );
        }

        return scope.Close(  Undefined() );
    }

    Handle<Value> Reply::AddDirEntry(const Arguments& args) {
        HandleScope scope;
        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 4) {
            ThrowException(Exception::TypeError(
                String::New("You must specify four arguments to invoke this function")));
            return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify an entry name String as first argument")));
            return scope.Close(Undefined());
        }

        if (!args[1]->IsNumber()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify the requested size number as second argument")));
            return scope.Close(Undefined());
        }

        if (!args[2]->IsObject()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify stat Object as third argument")));
            return scope.Close(Undefined());
        }

        if (!args[3]->IsNumber()) {
            ThrowException(Exception::TypeError(
                String::New("You must specify a offset number as fourth argument")));
            return scope.Close(Undefined());
        }

        String::Utf8Value name(args[0]->ToString());
        size_t requestedSize = args[1]->IntegerValue();
        off_t offset = args[3]->IntegerValue();
        reply-> dentry_offset = offset;

        char* buffer = reply->dentry_buffer;

        struct stat statbuff;
        ObjectToStat(args[2]->ToObject(), &statbuff);


        size_t acc_size = reply->dentry_acc_size;

        size_t len = fuse_add_direntry(reply-> request, NULL, 0, *name, &statbuff, 0);
        buffer = (char * )realloc(buffer, acc_size + len);
        while(!buffer){
            printf("failed to allocate buffer for direntry, retrying\n");
            buffer = (char * )realloc(buffer, acc_size + len);
        }         
        reply->dentry_buffer = buffer;
        size_t len2 = fuse_add_direntry(reply->request, (char*) (buffer + acc_size),
         requestedSize - acc_size,
         (const char*) *name, &statbuff, acc_size + len);
        
        // fprintf(stderr, "Current length! -> %d\n", (int)reply->dentry_cur_length);

        // fprintf(stderr, "Entry name -> %s\n", (const char*) *name);
        // fprintf(stderr, "Space needed for the entry -> %d or %d \n", (int) len, (int)len2);
        // fprintf(stderr, "Requested size -> %d\n", (int) requestedSize);
        // fprintf(stderr, "Remaning buffer -> %d\n", (int)(requestedSize - acc_size));
        // fprintf(stderr, "Offset -> %lld\n\n", offset );        

        reply->dentry_acc_size += len;
        reply->dentry_cur_length++;

        return scope.Close(Integer::New(len2));
    }
} //ends namespace NodeFuse
