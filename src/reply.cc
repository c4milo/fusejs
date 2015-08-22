/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include "reply.h"
#include "file_info.h"
#include "node_buffer.h"
#define MIN(x, y) ((x) < (y) ? (x) : (y))

namespace NodeFuse {
    Nan::Persistent<Function> Reply::constructor;
    void Reply::New(const Nan::FunctionCallbackInfo<v8::Value>& info){
        if (info.IsConstructCall()) {
          Reply *reply = new Reply();
          Local<Object> obj = info.This();
          reply->Wrap(obj);
          info.GetReturnValue().Set( obj );
        } else {
          Local<Function> cons = Nan::New<Function>(constructor);
          info.GetReturnValue().Set(cons->NewInstance());
        }

    }

    void Reply::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
        tpl->SetClassName(Nan::New<v8::String>("Reply").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        Nan::SetPrototypeMethod(tpl, "entry", Reply::Entry);
        Nan::SetPrototypeMethod(tpl, "attr", Reply::Attributes);
        Nan::SetPrototypeMethod(tpl, "readlink", Reply::ReadLink);
        Nan::SetPrototypeMethod(tpl, "err", Reply::Error);
        Nan::SetPrototypeMethod(tpl, "open", Reply::Open);
        Nan::SetPrototypeMethod(tpl, "buffer", Reply::Buffer);
        Nan::SetPrototypeMethod(tpl, "write", Reply::Write);
        Nan::SetPrototypeMethod(tpl, "statfs", Reply::StatFs);
        Nan::SetPrototypeMethod(tpl, "create", Reply::Create);
        Nan::SetPrototypeMethod(tpl, "xattr", Reply::XAttributes);
        Nan::SetPrototypeMethod(tpl, "addDirEntry", Reply::AddDirEntry);
        Nan::SetPrototypeMethod(tpl, "none", Reply::None);

        constructor.Reset(tpl->GetFunction());
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

    void Reply::Entry(const Nan::FunctionCallbackInfo<v8::Value>& args) {
        Nan::HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            Nan::ThrowTypeError( "You must specify an object as first argument");
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(arg, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
        }

        ret = fuse_reply_entry(reply->request, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Attributes(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            Nan::ThrowTypeError( "You must specify an object as first argument");
        }

        int ret = -1;
        struct stat statbuff;

        ret = ObjectToStat(arg, &statbuff);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized stat object: ", "Unable to reply the operation");
        }

        double timeout = 0;
        if (argslen == 2) {
            if (!args[1]->IsNumber()) {
                FUSEJS_THROW_EXCEPTION("Invalid timeout, ", "it should be the number of seconds in which the attributes are considered valid.");
            }

            timeout = args[1]->NumberValue();
        }

        ret = fuse_reply_attr(reply->request, &statbuff, timeout);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::ReadLink(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsString()) {
            Nan::ThrowTypeError( "You must specify a string as first argument");
        }

        String::Utf8Value link(arg->ToString());

        int ret = -1;
        ret = fuse_reply_readlink(reply->request, (const char*) *link);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Error(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            Nan::ThrowTypeError( "You must specify a number as first argument");
        }

        int ret = -1;
        ret = fuse_reply_err(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Open(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Object> fiobj = args[0]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            Nan::ThrowTypeError( "You must specify a FileInfo object as first argument");
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        int ret = -1;
        ret = fuse_reply_open(reply->request, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Buffer(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify 2 arguments to invoke this function");
        }

        if (!Buffer::HasInstance(args[0])) {
            Nan::ThrowTypeError( "You must specify a Buffer object as first argument");

        }
        if (!args[1]->IsNumber()) {
            Nan::ThrowTypeError( "You must specify the size of the buffer");

        }

        int ret = -1;
        size_t size = args[1]->IntegerValue();

        if (reply->dentry_buffer == NULL){
            // fprintf(stderr, "reply buf null\n");
            Local<Object> buffer = args[0]->ToObject();
            const char* data = Buffer::Data(buffer);
            ret = fuse_reply_buf( reply->request, data, size);
        }else{

            if (reply->dentry_offset < reply->dentry_acc_size){
                // fprintf(stderr, "reply buf less than %6d, %6d, \t%6d\n", (int) reply->dentry_offset, (int) reply->dentry_acc_size, (int) MIN(reply->dentry_acc_size - reply->dentry_offset,size));
                ret = fuse_reply_buf(reply->request, reply->dentry_buffer + reply->dentry_offset, MIN(reply->dentry_acc_size - reply->dentry_offset,size) );
            }else{
                // fprintf(stderr, "reply buf done\n");
                ret = fuse_reply_buf(reply->request, NULL, 0 );
            }

        }
        // fprintf(stderr, "reply buf return %d\n", ret);

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Write(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            Nan::ThrowTypeError( "You must specify the number of bytes written as first argument");
        }

        int ret = -1;
        ret = fuse_reply_write(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::StatFs(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int ret = -1;
        struct statvfs buf;

        int argslen = args.Length();

        if (argslen == 0) {
            Nan::ThrowTypeError( "You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            Nan::ThrowTypeError( "You must specify a object as first argument");
        }

        ret = ObjectToStatVfs(arg, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized statvfs object: ", "Unable to reply the operation");
        }

        ret = fuse_reply_statfs(reply->request, &buf);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Create(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 2) {
            Nan::ThrowTypeError( "You must specify at least two arguments to invoke this function");
        }

        Local<Value> params = args[0];
        if (!params->IsObject()) {
            Nan::ThrowTypeError( "You must specify an object as first argument");
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(params, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
        }

        Local<Object> fiobj = args[1]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            Nan::ThrowTypeError( "You must specify a FileInfo object as second argument");
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        ret = fuse_reply_create(reply->request, &entry, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::XAttributes(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError("You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            Nan::ThrowTypeError("You must specify a number as first argument");
        }

        int ret = -1;
        ret = fuse_reply_xattr(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::Lock(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            Nan::ThrowTypeError("You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            Nan::ThrowTypeError("You must specify a Lock object as first argument");
        }

        int ret = -1;
        struct flock lock;
        ret = ObjectToFlock(arg, &lock);

        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized lock object: ", "Unable to reply the operation");
        }

        ret = fuse_reply_lock(reply->request, &lock);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::None(const Nan::FunctionCallbackInfo<v8::Value>& args) {
        Nan::EscapableHandleScope scope;
        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);       
        fuse_reply_none(reply->request); 
    }


    void Reply::BMap(const Nan::FunctionCallbackInfo<v8::Value>& args){
        Nan::HandleScope scope;;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            Nan::ThrowTypeError("You must specify arguments to invoke this function");
        }

        Local<Value> arg = args[0];
        if (!arg->IsNumber()) {
            Nan::ThrowTypeError("You must specify a number as first argument");
        }

        int ret = -1;
        ret = fuse_reply_bmap(reply->request, arg->IntegerValue());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
        }

    }

    void Reply::AddDirEntry(const Nan::FunctionCallbackInfo<v8::Value>& args) {
        Nan::EscapableHandleScope scope;
        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0 || argslen < 4) {
            Nan::ThrowTypeError("You must specify four arguments to invoke this function");
        }

        if (!args[0]->IsString()) {
            Nan::ThrowTypeError("You must specify an entry name String as first argument");
        }

        if (!args[1]->IsNumber()) {
            Nan::ThrowTypeError("You must specify the requested size number as second argument");
        }

        if (!args[2]->IsObject()) {
            Nan::ThrowTypeError("You must specify stat Object as third argument");
        }

        if (!args[3]->IsNumber()) {
            Nan::ThrowTypeError("You must specify a offset number as fourth argument");
        }

        String::Utf8Value name(args[0]->ToString());
        size_t requestedSize = args[1]->IntegerValue();
        off_t offset = args[3]->IntegerValue();
        reply-> dentry_offset = offset;

        char* buffer = reply->dentry_buffer;

        struct stat statbuff;
        ObjectToStat(args[2]->ToObject(), &statbuff);
        // fprintf(stderr, "stat\n");

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
        // fprintf(stderr, "Actual Size -> %d\n", (int) acc_size );        

        reply->dentry_acc_size += len;
        reply->dentry_cur_length++;

        // scope.Escape(Nan::New<Number>( (int) len2) );
        args.GetReturnValue().Set(Nan::New<Number>( (int) len2));
    }
} //ends namespace NodeFuse
