--TEST--
V8Context class registration test
--SKIPIF--
<?php if (!extension_loaded('v8')) die('v8 not loaded'); ?>
--FILE--
<?php

class Dummy
{
    public $pub = 42;

    /*public function __construct()
    {
        echo __METHOD__;
    }
    */

}


$c = new V8Context();

var_dump($c->registerClass('Dummy'));

var_dump($c->run('d = new Dummy; d.pub = 43; d'));

var_dump($c->get('d'));


//$d = $c->get('d');

//var_dump($d);


echo 'ok';

?>
--EXPECT--
