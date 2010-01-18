--TEST--
V8Context object test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->run("a = { key: 'hello', k: 'w' }; a"));

var_dump($c->run("a = {}"));


$obj = new stdClass;
$obj->propa = 'vala';
$obj->propb = array(1,2,3.14,'key'=>'val');
echo 'before';

var_dump($c->set('obj', $obj));

var_dump($c->set('obj2', $obj));

var_dump($c->get('obj'));



var_dump($c->run('obj.propa'));

/*
unset($obj);

var_dump($c->run('obj.propb'));





var_dump($c->get('obj'));

*/
echo 'after';


?>
--EXPECT--
object(stdClass)#2 (2) {
  ["key"]=>
  string(5) "hello"
  ["k"]=>
  string(1) "w"
}
object(stdClass)#2 (0) {
}
beforebool(true)
bool(true)
object(stdClass)#2 (2) {
  ["propa"]=>
  string(4) "vala"
  ["propb"]=>
  array(4) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    float(3.14)
    ["key"]=>
    string(3) "val"
  }
}
NULL
after

