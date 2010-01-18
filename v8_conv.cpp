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

#include "v8_conv.h"
#include "v8_xref.h"
#include "v8_v8context.h"

using namespace v8;

Handle<Value> to_jsval(v8context* v8c, zval* val TSRMLS_DC)
{
    if (!val) {
        return Undefined();
    }
    switch(Z_TYPE_P(val)) {
    case IS_NULL:
        return Null();
    case IS_BOOL:
        return Boolean::New(Z_BVAL_P(val) ? true : false);
    case IS_LONG:
        return Integer::New(Z_LVAL_P(val));
    case IS_DOUBLE:
        return Number::New(Z_DVAL_P(val));
    case IS_STRING:
        return String::New(Z_STRVAL_P(val), Z_STRLEN_P(val));

    case IS_ARRAY:
        {
            Context::Scope context_scope(v8c->get_context());
            HashTable* ht = HASH_OF(val);
            Handle<Array> arr = Array::New(ht->nNumOfElements);
            for(zend_hash_internal_pointer_reset(ht); zend_hash_has_more_elements(ht) == SUCCESS
                ; zend_hash_move_forward(ht))
            {
                char*  key;
                uint   key_len;
                ulong  index;
                zval** data;
                int type = zend_hash_get_current_key_ex(ht, &key, &key_len, &index, 0, NULL);
                if (FAILURE == zend_hash_get_current_data(ht, (void**)&data)) {
                    continue;
                }
                if (type == HASH_KEY_IS_LONG) {
                    arr->Set(Integer::New(static_cast<double>(index)), to_jsval(v8c, *data TSRMLS_CC));
                }
                else {
                    arr->Set(String::New(key, key_len-1), to_jsval(v8c, *data TSRMLS_CC));
                }
            }
            return arr;
        }

    case IS_OBJECT:
        {
            Handle<ObjectTemplate> tpl = ObjectTemplate::New();
            tpl->SetInternalFieldCount(1);
            Handle<Object> obj = tpl->NewInstance();

            v8c->register_ze_obj(obj, val TSRMLS_CC);
            return obj;
        }

    default:
        break;
    }
    return Undefined();
}


void to_zval(zval* return_value, const Handle<Value>& jval TSRMLS_DC)
{
    if (jval->IsInt32()) {
        RETURN_LONG(jval->Int32Value());
    }
    else if (jval->IsNumber()) {
        RETURN_DOUBLE(jval->NumberValue());
    }
    else if (jval->IsString()) {
        String::AsciiValue ascii(jval);
        size_t ascii_len = strlen(*ascii);
        RETURN_STRINGL(estrndup(*ascii, ascii_len), ascii_len, 0);
    }
    else if (jval->IsBoolean()) {
        RETURN_BOOL(jval->IsFalse() ? 0 : 1);
    }

    else if (jval->IsArray()) {

        Handle<Array> arr = Handle<Array>::Cast(jval);
        array_init_size(return_value, arr->Length());

        Handle<Array> props = arr->GetPropertyNames();
        for (uint32_t i = 0; i<props->Length(); ++i) {

            zval* data;
            MAKE_STD_ZVAL(data);

            Local<Value> prop = props->Get(Integer::New(i));
            Local<Value> val = arr->Get(prop);

            to_zval(data, val TSRMLS_CC);

            if (prop->IsInt32()) {
                add_index_zval(return_value, static_cast<ulong>(prop->Uint32Value()), data);
            }
            else {
                String::AsciiValue ascii(prop);
                add_assoc_zval_ex(return_value, *ascii, ascii.length()+1, data);
            }
        }

        return;
    }

    // mettre IsFunction, IsDate avant IsObject() test
    else if (jval->IsObject()) {

        Handle<Object> obj = jval->ToObject();

        ze_obj* zeo = 0;

        if (obj->InternalFieldCount()) { // obj as a corresponding ZE object
            zeo = static_cast<ze_obj*>(Handle<External>::Cast(obj->GetInternalField(0))->Value());

            zval* php_obj = zeo->obj;
            if (php_obj) {
                zval_ptr_dtor(&return_value); // ????

                *return_value = *php_obj;
                return_value = php_obj;
                zval_copy_ctor(return_value);

                /*
                Context::Scope context_scope(zeo->context->get_context());

                Handle<Array> props = obj->GetPropertyNames();

                for (uint32_t i = 0; i<props->Length(); ++i) {
                    Local<Value> prop = props->Get(Integer::New(i));
                    Local<Value> propval = obj->GetRealNamedProperty(prop->ToString());
                    zval* data;
                    MAKE_STD_ZVAL(data);
                    to_zval(data, propval TSRMLS_CC);
                    String::AsciiValue ascii(prop);
                    zend_update_property(zeo->ce, return_value, *ascii, ascii.length(), data TSRMLS_CC);
                    //zval_ptr_dtor(&data); ///?
                }
                */
                return;
            }
        }

        object_init(return_value);
        Handle<Array> props = obj->GetPropertyNames();
        for (uint32_t i = 0; i<props->Length(); ++i) {
            Local<Value> prop = props->Get(Integer::New(i));
            Local<Value> propval = obj->Get(prop);
            zval* data;
            MAKE_STD_ZVAL(data);
            to_zval(data, propval TSRMLS_CC);
            String::AsciiValue ascii(prop);
            zend_update_property(0, return_value, *ascii, ascii.length(), data TSRMLS_CC);
            zval_ptr_dtor(&data); ///?
        }
        return;
    }

    RETVAL_NULL();

}
