--TEST--
V8Context class creation test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php 
$c = new V8Context();
var_dump($c);

?>
--EXPECT--
object(V8Context)#1 (0) {
}
