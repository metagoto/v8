--TEST--
V8Context array test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->run("a = [1,2,'hello',3.14, false,true,null]; a.zz = true; a"));



?>
--EXPECT--
array(8) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  string(5) "hello"
  [3]=>
  float(3.14)
  [4]=>
  bool(false)
  [5]=>
  bool(true)
  [6]=>
  NULL
  ["zz"]=>
  bool(true)
}

