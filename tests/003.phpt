--TEST--
V8Context get/set/exists test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->set('a', 1));
var_dump($c->get('a'));
var_dump($c->exists('a'));

var_dump($c->get('b'));

var_dump($c->exists('c'));

var_dump($c->run("a = 42"));
var_dump($c->exists('a'));
var_dump($c->get('a'));




?>
--EXPECT--
bool(true)
int(1)
bool(true)
NULL
bool(false)
int(42)
bool(true)
int(42)
