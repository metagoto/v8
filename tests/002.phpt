--TEST--
V8Context simple runs test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->run("1 + 1"));
var_dump($c->run("'hello ' + 'world'"));
var_dump($c->run("1.3 + 1.4"));
var_dump($c->run("true"));
var_dump($c->run("null"));

var_dump($c->run("a = 42"));
var_dump($c->run("a"));



?>
--EXPECT--
int(2)
string(11) "hello world"
float(2.7)
bool(true)
NULL
int(42)
int(42)

