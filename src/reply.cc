// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "reply.h"
#include "file_info.h"

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

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("Reply"));
    }

    Reply::Reply() : ObjectWrap() {}
    Reply::~Reply() {}

    Handle<Value> Reply::Entry(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an object as first argument")));
        }

        int ret = -1;
        struct fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(arg, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            return Null();
        }

        ret = fuse_reply_entry(reply->request, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return Null();
        }

        return Undefined();
    }

    Handle<Value> Reply::Attributes(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];
        if (!arg->IsObject()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an object as first argument")));
        }

        int ret = -1;
        struct stat statbuff;

        ret = ObjectToStat(arg, &statbuff);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized stat object: ", "Unable to reply the operation");
            return Null();
        }

        double timeout = 0;
        if (argslen == 2) {
            if (!args[1]->IsNumber()) {
                FUSEJS_THROW_EXCEPTION("Invalid timeout, ", "it should be the number of seconds in which the attributes are considered valid.");
                return Null();
            }

            timeout = args[1]->NumberValue();
        }

        ret = fuse_reply_attr(reply->request, &statbuff, timeout);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return Null();
        }

        return Undefined();
    }

    Handle<Value> Reply::ReadLink(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];
        if (!arg->IsString()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify a string as first argument")));
        }

        String::Utf8Value link(arg->ToString());

        int ret = -1;
        ret = fuse_reply_readlink(reply->request, (const char*) *link);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return Null();
        }

        return Undefined();
    }

    Handle<Value> Reply::Error(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];
        if (!arg->IsInt32()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify a number as first argument")));
        }

        int ret = -1;
        ret = fuse_reply_err(reply->request, arg->Int32Value());
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return Null();
        }

        return Undefined();
    }

    Handle<Value> Reply::Open(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply* reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();
        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Object> fiobj = args[0]->ToObject();
        if (!FileInfo::HasInstance(fiobj)) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify a FileInfo object as first argument")));
        }

        FileInfo* fileInfo = ObjectWrap::Unwrap<FileInfo>(fiobj);

        int ret = -1;
        ret = fuse_reply_open(reply->request, fileInfo->fi);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            return Null();
        }

        return Undefined();
    }
} //ends namespace NodeFuse
