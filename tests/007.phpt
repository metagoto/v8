--TEST--
V8Context register function  test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php
$c = new V8Context();

var_dump($c->registerFunction('printf'));

var_dump($c->run('printf("int: %d, str: %s", [].length, {k:1, v:2}.toString())'));

/*var_dump($c->run('fun("ZZ")'));


var_dump($c->registerFunction('fun2', function($a, $b = "def2"){ var_dump($a); var_dump($b); return 42; }));

var_dump($c->run('fun2(["hello", "world"])'));

var_dump($c->run('a = ["hello", "world", 1, 3.14, null, true]; a.keyx="valx"; fun2(a, { key: "val!", key2: 2.618 })'));


var_dump($c->set('fun3', function($a = 'fun_3'){ echo "a: $a"; }));

var_dump($c->run('fun3(1,2); a = {k:[/bc/]};'));

var_dump($c->run('funNone(42);'));

var_dump($c->unset('fun3'));


$fun4 = function($a = 'fun_4'){ echo "a: $a"; };
var_dump($c->set('fun4', $fun4));

var_dump($c->run('fun4();'));

var_dump($c->unset('fun4'));

$fun4();
 */

echo 'ok';

?>
--EXPECT--
bool(true)
int: 0, str: [object Object]int(28)
ok
