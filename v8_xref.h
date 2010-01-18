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

#ifndef V8_XREF_H
#define V8_XREF_H


#include "php_v8.h"

struct v8context;


struct fcall_info
{

    fcall_info(const zend_fcall_info& fci, const zend_fcall_info_cache& fcc, v8context* context)
        : fci(fci)
        , fcc(fcc)
        , context(context)
    {
    }

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    v8context* context;
};


struct ze_obj
{

    ze_obj(zend_class_entry* ce, v8context* context, zval* obj = 0)
        : ce(ce)
        , context(context)
        , obj(obj)
    {
    }

    zend_class_entry* ce;
    v8context* context;
    zval* obj;
};


#endif // V8_XREF_H
