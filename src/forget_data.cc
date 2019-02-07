#include "forget_data.h"
#if FUSE_VERSION > 28

namespace NodeFuse {

	Nan::Persistent<Function> ForgetData::constructor;

	NAN_METHOD(ForgetData::New){
	    if (info.IsConstructCall()) {
	      ForgetData *fi = new ForgetData();
	      Local<Object> obj = info.This();
	      fi->Wrap(obj);
	      info.GetReturnValue().Set( obj );
	    } else {
	      Local<Function> cons = Nan::New<Function>(constructor);
	      info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
            //info.GetReturnValue().Set(cons->NewInstance());
	    }

	}
	void ForgetData::Initialize(Handle<Object> target){
		Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
		tpl->SetClassName(Nan::New<v8::String>("ForgetData").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		Local<ObjectTemplate> object_tmpl = tpl->InstanceTemplate();

		object_tmpl->SetInternalFieldCount(1);

		Nan::SetAccessor(object_tmpl, Nan::New("inode").ToLocalChecked(),   ForgetData::GetIno);
		Nan::SetAccessor(object_tmpl, Nan::New("nlookup").ToLocalChecked(), ForgetData::GetNLookup);
		constructor.Reset(tpl->GetFunction());

	}
	ForgetData::ForgetData() : Nan::ObjectWrap() {}
	ForgetData::~ForgetData() {}
	void ForgetData::GetIno(v8::Local<v8::String> property,
                const Nan::PropertyCallbackInfo<v8::Value>& info) 
	{
		ForgetData *forget_data = Nan::ObjectWrap::Unwrap<ForgetData>(info.This());
		info.GetReturnValue().Set( Nan::New<Number>((double)(forget_data->fd.ino)));
	}

	void ForgetData::GetNLookup(v8::Local<v8::String> property,
                const Nan::PropertyCallbackInfo<v8::Value>& info) 
	{
		ForgetData *forget_data = Nan::ObjectWrap::Unwrap<ForgetData>(info.This());
		info.GetReturnValue().Set( Nan::New<Number>( (double)(forget_data->fd.nlookup)) );
	}

}
#endif
