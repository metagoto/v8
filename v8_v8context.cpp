/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: metagoto <runpac314 gmail com>                              |
   +----------------------------------------------------------------------+
 */

#include "v8_v8context.h"
#include "v8_conv.h"

#include <iostream>
#include <fstream>

using namespace v8;


v8context::v8context()
{
    HandleScope handle_scope;
    Local<ObjectTemplate> global = ObjectTemplate::New();
    context = Context::New(NULL, global);
}


v8context::~v8context()
{
    std::for_each(fcall_list.begin(), fcall_list.end(), del_fcall_info());
    std::for_each(ze_obj_list.begin(), ze_obj_list.end(), del_ze_obj());

    context.Dispose();
}


Handle<Value> v8context::run(const char* src, const int len)
{

    HandleScope handle_scope;
    Context::Scope context_scope(context);

    TryCatch trycatch;
    Handle<Script> script = Script::Compile(String::New(src, len));

    if (script.IsEmpty()) {
        return handle_scope.Close(trycatch.Exception()->ToString());
    }

    Local<Value> result = script->Run();
    if (result.IsEmpty()) {
        String::AsciiValue exception(trycatch.Exception());
        return handle_scope.Close(trycatch.Exception()->ToString());
    }

    return handle_scope.Close(result);
}


void v8context::set(const char* name, const int name_len, Handle<Value> value)
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);
    context->Global()->Set(String::New(name, name_len), value);
}


Handle<Value> v8context::get(const char* name, const int name_len) const
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);
    return handle_scope.Close(context->Global()->Get(String::New(name, name_len)));
}


bool v8context::has(const char* name, const int name_len) const
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);
    return context->Global()->Has(String::New(name, name_len));
}


bool v8context::unset(const char* name, const int name_len)
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);

    Local<Value> prop = context->Global()->Get(String::New(name, name_len));

    if (prop.IsEmpty() || !prop->IsFunction()) {
        return false;
    }

    Local<Function> func = Local<Function>::Cast(prop);
    Local<Value> val = func->GetHiddenValue(String::New("p"));
    if (!val.IsEmpty()) {
        Handle<External> field = Handle<External>::Cast(val);
        fcall_info* fc = static_cast<fcall_info*>(field->Value());

        fcall_list_t::iterator it = std::find(fcall_list.begin(), fcall_list.end(), fc);
        if (it != fcall_list.end()) {
            fcall_list.erase(it);
            del_fcall_info()(fc);
            context->Global()->ForceDelete(String::New(name, name_len));
            return true;
        }
    }
    return false;
}


const Persistent<Context>& v8context::get_context() const
{
    return context;
}


void v8context::register_ze_obj(Handle<Object> obj, zval* val TSRMLS_DC)
{
    ze_obj* zeo = 0;
    ze_obj_list_t::iterator it = std::find_if(ze_obj_list.begin(), ze_obj_list.end(), ze_obj_zval_pred(val));
    if (it == ze_obj_list.end()) {
        Z_ADDREF_P(val);
        zeo = new ze_obj(Z_OBJCE_P(val), this, val);
        ze_obj_list.push_back(zeo);
    }
    else {
        zeo = *it;
    }
    obj->SetInternalField(0, External::New(zeo));
}


void v8context::register_func(const char* name, const int name_len, const zend_fcall_info& fci, const zend_fcall_info_cache& fcc)
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);

    fcall_info* fc = new fcall_info(fci, fcc, this);
    fcall_list.push_back(fc);

    Handle<FunctionTemplate> temp = FunctionTemplate::New(func_callback);
    Local<Function> func = temp->GetFunction();
    func->SetHiddenValue(String::New("p"), External::New(fc));

    context->Global()->Set(String::New(name, name_len), func);
}


/*Handle<Value> interceptor_get(Local<String> name, const AccessorInfo& args)
{
    php_printf("INTER_GET\n");
    return Null();
}

Handle<Value> interceptor_set(Local<String> name, Local<Value> value, const AccessorInfo& args)
{
    php_printf("INTER_SET\n");
    return Null();
}
*/

void v8context::register_class(const char* name, const int name_len, zend_class_entry* ce)
{
    HandleScope handle_scope;
    Context::Scope context_scope(context);

    reg_class_pack* pack = new reg_class_pack(ce, this);

    Local<FunctionTemplate> func = FunctionTemplate::New(&ctor_callback, External::New(static_cast<void*>(pack)));
    func->SetClassName(String::New(name, name_len));

    Local<ObjectTemplate> obj_tpl = func->InstanceTemplate();
    obj_tpl->SetNamedPropertyHandler(&interceptor_get, interceptor_set); // see the rest of args!
    obj_tpl->SetInternalFieldCount(1);

    context->Global()->Set(v8::String::New(name, name_len), func->GetFunction());
}



Handle<Value> v8context::ctor_callback(const Arguments& args)
{
    TSRMLS_FETCH();

    HandleScope handle_scope;

    reg_class_pack* pack = static_cast<reg_class_pack*>(Handle<External>::Cast(args.Data())->Value());

    v8context* self = pack->context;
    zend_class_entry* ce = pack->ce;

    zval* obj;
    MAKE_STD_ZVAL(obj);

    //if (ce->constructor) {

    //}
    //else {
    object_init_ex(obj, ce);
    //}

    //Z_ADDREF_P(obj); // ???

    ze_obj* zeo = new ze_obj(ce, self, obj);
    self->ze_obj_list.push_back(zeo);

    args.This()->SetInternalField(0, External::New(static_cast<void*>(zeo)));

    delete pack;
    return args.This();
}


Handle<Value> v8context::getter(Local<String> name, const AccessorInfo& args, ze_obj* zeo)
{
    TSRMLS_FETCH();

    Handle<Object> obj = args.This();

    if (obj->HasRealNamedProperty(name)) {
        return obj->GetRealNamedProperty(name);
    }

    String::AsciiValue prop(name);
    zval* val = zend_read_property(zeo->ce, zeo->obj, *prop, prop.length(), 1 TSRMLS_CC);
    if (val) {
        Handle<Value> jsval = to_jsval(this, val TSRMLS_CC);
        zval_ptr_dtor(&val);
        return jsval;
    }

    return Null();
}


Handle<Value> v8context::setter(Local<String> name, Local<Value> value, const AccessorInfo& args, ze_obj* zeo)
{
    TSRMLS_FETCH();

    Handle<Object> obj = args.This();

    zval* data;
    MAKE_STD_ZVAL(data);
    to_zval(data, value TSRMLS_CC);
    String::AsciiValue ascii(name);

    PHPWRITE(*ascii, ascii.length());

    zend_update_property(zeo->ce, zeo->obj, *ascii, ascii.length(), data TSRMLS_CC);

    //args.This()->GetRealNamedProperty(String::New("test"));
    /////////args.This()->Set(name, value);

    /*zval* val = zend_read_property(zeo->ce, zeo->obj, *prop, prop.length(), 1 TSRMLS_CC);
    if (val) {
        v8::Handle<v8::Value> jsval = to_jsval(this, val TSRMLS_CC);
        zval_ptr_dtor(&val);
        return jsval;
    }
    php_printf("RETURN_NULL\n");
    */
    return value; //Null();
}



Handle<Value> v8context::func_callback(const Arguments& args)
{
    TSRMLS_FETCH();

    HandleScope handle_scope;

    Handle<External> field = Handle<External>::Cast(args.Callee()->GetHiddenValue(String::New("p")));
    fcall_info* fc = static_cast<fcall_info*>(field->Value());

    v8context* self = fc->context;
    Context::Scope context_scope(self->context);

    int argc = args.Length();

    zval*** params = (zval***)emalloc(argc * sizeof(zval**));
    for (int i = 0; i < argc; ++i) {
        zval** val = (zval**)emalloc(sizeof(zval*));
        MAKE_STD_ZVAL(*val);
        to_zval(*val, args[i] TSRMLS_CC);
        params[i] = val;
    }

    zval* retval = 0;
    fc->fci.params         = params;
    fc->fci.param_count    = argc;
    fc->fci.retval_ptr_ptr = &retval;
    zend_call_function(&fc->fci, NULL TSRMLS_CC);

    for (int i = 0; i < argc; ++i) {
        zval** val = params[i];
        zval_ptr_dtor(val);
        efree(val);
    }
    efree(params);

    if (retval != NULL) {
        Handle<Value> val = to_jsval(self, retval TSRMLS_CC);
        zval_ptr_dtor(&retval);
        return handle_scope.Close(val);
    }
    return Null();
}


