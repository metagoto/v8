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

#ifndef V8_V8CONTEXT_H
#define V8_V8CONTEXT_H


#include <list>
#include <functional>
#include <algorithm>
#include <v8.h>
#include "php_v8.h"
#include "v8_xref.h"


struct v8context;


struct reg_class_pack
{
    reg_class_pack(zend_class_entry* ce, v8context* context)
        : ce(ce)
        , context(context)
    {}

    zend_class_entry* ce;
    v8context* context;
};


struct v8context
{

    v8context();

    ~v8context();

    v8::Handle<v8::Value> run(const char* src, const int len);

    void set(const char* name, const int name_len, v8::Handle<v8::Value> value);

    v8::Handle<v8::Value> get(const char* name, const int name_len) const;

    bool has(const char* name, const int name_len) const;

    bool unset(const char* name, const int name_len);


    void register_func(const char* name, const int name_len, const zend_fcall_info& fci, const zend_fcall_info_cache& fcc);

    void register_class(const char* name, const int name_len, zend_class_entry* ce);


    v8::Handle<v8::Value> getter(v8::Local<v8::String> name, const v8::AccessorInfo& args, ze_obj* zeo);
    v8::Handle<v8::Value> setter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& args, ze_obj* zeo);


    static v8::Handle<v8::Value> func_callback(const v8::Arguments& args);

    static v8::Handle<v8::Value> ctor_callback(const v8::Arguments& args);


    static v8::Handle<v8::Value> interceptor_get(v8::Local<v8::String> name, const v8::AccessorInfo& args)
    {
        ze_obj* zeo = static_cast<ze_obj*>(v8::Handle<v8::External>::Cast(args.This()->GetInternalField(0))->Value());
        return zeo->context->getter(name, args, zeo);
    }

    static v8::Handle<v8::Value> interceptor_set(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& args){
        ze_obj* zeo = static_cast<ze_obj*>(v8::Handle<v8::External>::Cast(args.This()->GetInternalField(0))->Value());
        return zeo->context->setter(name, value, args, zeo);
    }


    //static void release(v8::Persistent<v8::Value> val, void* parameter);


    const v8::Persistent<v8::Context>& get_context() const;


    void register_ze_obj(v8::Handle<v8::Object> obj, zval* val TSRMLS_DC);



    struct del_fcall_info : public std::unary_function<void, fcall_info*>
    {
        void operator()(fcall_info* fc) {
            if (fc->fci.function_name) {
                zval_ptr_dtor(&fc->fci.function_name);
            }
            delete fc;
        }
    };

    struct del_ze_obj : public std::unary_function<void, ze_obj*>
    {
        void operator()(ze_obj* zeo) {
            if (zeo->obj) {
                zval_ptr_dtor(&zeo->obj);
            }
            delete zeo;
        }
    };

    struct ze_obj_zval_pred : public std::unary_function<bool, ze_obj*>
    {
        ze_obj_zval_pred(zval* val)
            : val(val)
        {}
        bool operator()(ze_obj* zeo) const
        {
            return zeo->obj == val;
        }
        zval* val;
    };


private:

    v8::Persistent<v8::Context> context;

    typedef std::list<fcall_info*> fcall_list_t;
    fcall_list_t fcall_list;

    typedef std::list<ze_obj*> ze_obj_list_t;
    ze_obj_list_t ze_obj_list;

};


#endif // V8_V8CONTEXT_H
