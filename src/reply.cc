// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "reply.h"

namespace NodeFuse {
    Persistent<FunctionTemplate> Reply::constructor_template;

    void Reply::Initialize() {
        Local<FunctionTemplate> t = FunctionTemplate::New();

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "entry", Reply::Entry);
        NODE_SET_PROTOTYPE_METHOD(t, "attr", Reply::Attributes);

        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->SetClassName(String::NewSymbol("Reply"));
    }

    Reply::Reply() : ObjectWrap() {

    }

    Reply::~Reply() {}

    Handle<Value> Reply::Entry(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply *reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];

        if (!arg->IsObject() && !arg->IsInt32()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an object or number as first argument")));
        }

        int ret = -1;
        if (arg->IsInt32()) {
            ret = fuse_reply_err(reply->request, arg->Int32Value());
            if (ret == -1) {
                FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            }
            return Null();
        }

        fuse_entry_param entry;

        ret = ObjectToFuseEntryParam(arg, &entry);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized fuse entry structure: ", "Unable to reply the operation");
            return Null();
        }

        fuse_reply_entry(reply->request, &entry);
    }

    Handle<Value> Reply::Attributes(const Arguments& args) {
        HandleScope scope;

        Local<Object> replyObj = args.This();
        Reply *reply = ObjectWrap::Unwrap<Reply>(replyObj);

        int argslen = args.Length();

        if (argslen == 0) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify arguments to invoke this function")));
        }

        Local<Value> arg = args[0];

        if (!arg->IsObject() && !arg->IsInt32()) {
            return ThrowException(Exception::TypeError(
            String::New("You must specify an object or number as first argument")));
        }

        int ret = -1;
        if (arg->IsInt32()) {
            ret = fuse_reply_err(reply->request, arg->Int32Value());
            if (ret == -1) {
                FUSEJS_THROW_EXCEPTION("Error replying operation: ", strerror(errno));
            }
            return Null();
        }

        struct stat statbuff;

        ret = ObjectToStat(arg, &statbuff);
        if (ret == -1) {
            FUSEJS_THROW_EXCEPTION("Unrecognized stat object: ", "Unable to reply the operation");
            return Null();
        }

        fuse_reply_attr(reply->request, &statbuff, 0); //grab timeout from js object
    }

} //ends namespace NodeFuse
