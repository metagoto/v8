--TEST--
V8Context callable test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->registerFunction(function($a = 'def'){ echo "a: $a"; }, 'fun'));

var_dump($c->run('fun()'));

var_dump($c->run('fun("ZZ")'));



var_dump($c->registerFunction(function($a, $b = "def2"){ var_dump($a); var_dump($b); return 42; }, 'fun2'));

var_dump($c->run('fun2(["hello", "world"])'));

var_dump($c->run('a = ["hello", "world", 1, 3.14, null, true]; a.keyx="valx"; fun2(a, { key: "val!", key2: 2.618 })'));



var_dump($c->set('fun3', function($a = 'fun_3'){ echo "a: $a"; })); //, V8Context::PHP_FUNCTION));

var_dump($c->run('fun3(1,2); a = {k:[/bc/]};'));

var_dump($c->run('funNone(42);'));

var_dump($c->unset('fun3'));


$fun4 = function($a = 'fun_4'){ echo "a: $a"; };
var_dump($c->set('fun4', $fun4, V8Context::PHP_FUNCTION));

var_dump($c->run('fun4();'));

var_dump($c->unset('fun4'));

$fun4();

echo 'ok';

?>
--EXPECT--
bool(true)
a: defNULL
a: ZZNULL
bool(true)
array(2) {
  [0]=>
  string(5) "hello"
  [1]=>
  string(5) "world"
}
string(4) "def2"
int(42)
array(7) {
  [0]=>
  string(5) "hello"
  [1]=>
  string(5) "world"
  [2]=>
  int(1)
  [3]=>
  float(3.14)
  [4]=>
  NULL
  [5]=>
  bool(true)
  ["keyx"]=>
  string(4) "valx"
}
object(stdClass)#4 (2) {
  ["key"]=>
  string(4) "val!"
  ["key2"]=>
  float(2.618)
}
int(42)
bool(true)
a: 1object(stdClass)#5 (1) {
  ["k"]=>
  array(1) {
    [0]=>
    object(stdClass)#6 (0) {
    }
  }
}
string(38) "ReferenceError: funNone is not defined"
bool(true)
bool(true)
a: fun_4NULL
bool(true)
a: fun_4ok
