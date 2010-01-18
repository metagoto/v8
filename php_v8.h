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

#ifndef PHP_V8_H
#define PHP_V8_H


#include "v8_php_api.h"


#ifdef ZTS
#define V8_G(v) TSRMG(v8_globals_id, zend_v8_globals *, v)
#else
#define V8_G(v) (v8_globals.v)
#endif

extern zend_module_entry v8_module_entry;
#define phpext_v8_ptr &v8_module_entry


#endif // PHP_V8_H
