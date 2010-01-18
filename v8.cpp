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


#include <v8.h>

#include "php_v8.h"
extern "C" {
#include "zend_closures.h"
}

#include "v8_v8context.h"
#include "v8_conv.h"



extern "C" {
#ifdef COMPILE_DL_V8
ZEND_GET_MODULE(v8)
#endif
} // extern "C"




static zend_class_entry* v8context_ce;
static zend_object_handlers v8context_object_handlers;


struct v8option
{
    enum attribute_type {
        ATTR_ERRMODE
    };

    enum error_mode_type {
        ERRMODE_SILENT,
        ERRMODE_WARNING,
        ERRMODE_EXCEPTION
    };

    enum php_type {
        PHP_VALUE,
        PHP_FUNCTION,
        PHP_CLASS
    };
};


struct v8context_object {
    zend_object std;
    v8context*  intern;
};



static void v8_object_dtor(void* object, zend_object_handle handle TSRMLS_DC)
{
    v8context_object* obj = (v8context_object*)object;

    delete obj->intern;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
    efree(obj);
}


zend_object_value v8_create_object(zend_class_entry* class_type TSRMLS_DC)
{
    zend_object_value retval;

    v8context_object* obj = (v8context_object*)emalloc(sizeof(v8context_object));
    memset(obj, 0, sizeof(v8context_object));

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    zend_hash_copy(obj->std.properties, &class_type->default_properties
                   ,reinterpret_cast<copy_ctor_func_t>(zval_add_ref), static_cast<void*>(0), sizeof(zval*));

    obj->intern = new v8context;


    retval.handle = zend_objects_store_put(obj, reinterpret_cast<zend_objects_store_dtor_t>(zend_objects_destroy_object)
                                              , reinterpret_cast<zend_objects_free_object_storage_t>(v8_object_dtor)
                                              , NULL TSRMLS_CC);
    retval.handlers = &v8context_object_handlers;

    return retval;
}



PHP_METHOD(V8Context, __construct)
{

}



PHP_METHOD(V8Context, run)
{
    char* src = NULL;
    int   src_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &src, &src_len)) {
        return;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    v8::HandleScope handle_scope;
    v8::Context::Scope context_scope(v8c->get_context());
    v8::Handle<v8::Value> result = v8c->run(src, src_len);

    to_zval(return_value, result TSRMLS_CC);
}


PHP_METHOD(V8Context, set)
{
    char* name = NULL;
    int   name_len = 0;
    zval* val = NULL;
    long  val_type = v8option::PHP_VALUE;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|l", &name, &name_len, &val, &val_type)
        || !name_len) {
        RETURN_FALSE;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    v8::HandleScope handle_scope;
    v8::Context::Scope context_scope(v8c->get_context());

    if (Z_TYPE_P(val) == IS_OBJECT && Z_OBJCE_P(val) == zend_ce_closure) {
        val_type = v8option::PHP_FUNCTION;
    }

    switch (val_type) {

    case v8option::PHP_VALUE: {
            v8::Handle<v8::Value> jsval = to_jsval(v8c, val TSRMLS_CC);
            v8c->set(name, name_len, jsval);
            RETURN_TRUE;
        }
    case v8option::PHP_FUNCTION: {
            zend_fcall_info fci;
            zend_fcall_info_cache fcc;

            if (SUCCESS == zend_fcall_info_init(val, 0, &fci, &fcc, NULL, NULL TSRMLS_CC)) {
                if (fci.function_name) {
                    Z_ADDREF_P(fci.function_name);
                }
                v8c->register_func(name, name_len, fci, fcc);

                RETURN_TRUE;
            }
            break;
        }
    case v8option::PHP_CLASS: {
            //stub
        }
    default:
        ;
    }

    RETURN_FALSE; // throw, warn... ?
}


PHP_METHOD(V8Context, registerFunction)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    char* name = NULL;
    int   name_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|s", &fci, &fcc, &name, &name_len)) {
        RETURN_FALSE;
    }

    if (!name_len) {
        name = Z_STRVAL_P(fci.function_name);
        name_len = Z_STRLEN_P(fci.function_name);
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    if (fci.function_name) {
        Z_ADDREF_P(fci.function_name);
    }
    v8c->register_func(name, name_len, fci, fcc);

    RETURN_TRUE;
}


PHP_METHOD(V8Context, registerClass)
{
    char* name = NULL;
    int   name_len = 0;
    char* js_name = NULL;
    int   js_name_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &name, &name_len, &js_name, &js_name_len)
        || !name_len) {
        RETURN_FALSE; // should throw if !name_len ?
    }

    zend_class_entry** ce = NULL;

    if (zend_lookup_class(name, name_len, &ce TSRMLS_CC) == FAILURE) {
        RETURN_FALSE;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    if (js_name_len) {
        v8c->register_class(js_name, js_name_len, *ce);
    }
    else {
        v8c->register_class(name, name_len, *ce);
    }
    RETURN_TRUE;
}


PHP_METHOD(V8Context, get)
{
    char* name = NULL;
    int   name_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)
        || !name_len) {
        RETURN_FALSE;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    v8::HandleScope handle_scope;
    v8::Context::Scope context_scope(v8c->get_context());
    to_zval(return_value, v8c->get(name, name_len) TSRMLS_CC);
}


PHP_METHOD(V8Context, exists)
{
    char* name = NULL;
    int   name_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)
        || !name_len) {
        RETURN_FALSE;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    if (v8c->has(name, name_len)) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


PHP_METHOD(V8Context, unset)
{
    char* name = NULL;
    int   name_len = 0;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)
        || !name_len) {
        RETURN_FALSE;
    }

    v8context_object* obj = (v8context_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
    v8context* v8c = obj->intern;

    if (v8c->unset(name, name_len)) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}


zend_function_entry v8context_methods[] = {
    PHP_ME(V8Context, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(V8Context, run, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, set, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, get, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, exists, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, unset, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, registerFunction, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(V8Context, registerClass, NULL, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};


/*const zend_function_entry splclassloader_functions[] = {
    {NULL, NULL, NULL}
};*/


PHP_MINIT_FUNCTION(v8)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "V8Context", v8context_methods);
    v8context_ce = zend_register_internal_class(&ce TSRMLS_CC);
    v8context_ce->create_object = v8_create_object;

    memcpy(&v8context_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    v8context_object_handlers.clone_obj = NULL; /* no cloning! */

    zend_declare_class_constant_long(v8context_ce, "PHP_VALUE",   sizeof("PHP_VALUE")-1,   static_cast<long>(v8option::PHP_VALUE) TSRMLS_CC);
    zend_declare_class_constant_long(v8context_ce, "PHP_FUNCTION",sizeof("PHP_FUNCTION")-1,static_cast<long>(v8option::PHP_FUNCTION) TSRMLS_CC);
    zend_declare_class_constant_long(v8context_ce, "PHP_CLASS",   sizeof("PHP_CLASS")-1,   static_cast<long>(v8option::PHP_CLASS) TSRMLS_CC);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(v8)
{
    php_info_print_table_start ();
    php_info_print_table_header(2, "v8 support", "enabled");
    php_info_print_table_row   (2, "libv8 version", v8::V8::GetVersion());
    php_info_print_table_end   ();
}


zend_module_entry v8_module_entry = {
    STANDARD_MODULE_HEADER,
    "v8",
    NULL, /*functions, */
    PHP_MINIT(v8),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(v8),
    "0.1",
    STANDARD_MODULE_PROPERTIES
};

