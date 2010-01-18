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

#ifndef V8_CONV_H
#define V8_CONV_H


#include "php_v8.h"

#include <v8.h>

struct v8context;


v8::Handle<v8::Value> to_jsval(v8context* v8c, zval* val TSRMLS_DC);


void to_zval(zval* return_value, const v8::Handle<v8::Value>& jval TSRMLS_DC);

#endif // V8_CONV_H
