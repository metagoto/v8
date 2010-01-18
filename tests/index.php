<?php
echo '<pre>';

//v8_test();

$c = new V8Context();

$c->registerFunction('printf');
$c->registerFunction('printf', 'printf_alias');


class A {
  public function get($p) {
    return __METHOD__.' with arg: '.$p;
  }

  public static function getStatic($p) {
    return __METHOD__.' with arg: '.$p;
  }

}

function free_fun($p) {
  return __FUNCTION__.' with arg: '.$p;
}

$lambda = function($p) {
  return __FUNCTION__.' with arg: '.$p;
};



$c->registerFunction('printf');
$c->registerFunction('printf', 'printf_alias');

var_dump($c->run('printf("str %s num %d\n", "hello world", 42)'));
var_dump($c->run('printf_alias("str %s num %d\n", "hello world", 42)'));


$a = new A;
$c->registerFunction(array($a, 'get'), 'a_get');
$c->registerFunction(array('A', 'getStatic'), 'a_getStatic');

var_dump($c->run('a_get(1) + " " + a_getStatic(3.14)'));


$c->registerFunction($lambda, 'lambda');
$c->registerFunction(function($p) { return 'from lambda_inplace with arg '.$p; }, 'lambda_inplace');

var_dump($c->run('lambda("str") + " " + lambda_inplace(69)'));

var_dump($c->exists('lambda'));
var_dump($c->unset('lambda'));
var_dump($c->exists('lambda'));

//var_dump($c->run('lambda("str") + " " + lambda_inplace(69)'));

function free_args() {
  var_dump(func_get_args());
}

$c->registerFunction('free_args');
var_dump($c->run('free_args("str", 11, 22.2, true, false, null)'));


$d = new V8Context();

$src =<<<EOS
a = new Array(3);
EOS;
var_dump($d->run($src));

$src =<<<EOS
a = new Array(3);
a[2] = 'ok';
a.dum = 'okdum';
a[7] = '7th';
a[9] = [true, false, 3.14, null, "str", []];
a
EOS;
var_dump($d->run($src));


$d->set('arrx', array('aww', 1, 'b', true, 'key'=>'keyval', 4.36, array("cp"=>"do_it", "other"=>"killer")));
var_dump($d->get('arrx'));


// js routes definition
$routes_def = <<<EOS
routes = {

  category: {
    pat: /^category(?:\/(\d+)(?:\/page\/(\d+))?)?$/,
    map: {1: "id", 2: "page" }
  },

  about: {
    pat: /^about$/
  },

  index: {
    pat: /^$/
  }

}
EOS;

// js router
$router = <<<EOS
(function(routes) {
  var res = { route: null, params: [] };
  for (var i in routes) {
    var match, route = routes[i];
    if (match = path_info.match(route.pat)) {
      res.route = i;
      if (route.map) {
        for (var k in route.map)
          res.params[route.map[k]] = match[k];
      }
      return setRoute(res);
    }
  }
  return false;
})(routes);
EOS;

class WebRequest {
  protected $route;
  public function setRoute($route) {
    $this->route = $route;

  }
}


// populate context
$e = new V8Context();
$e->run($routes_def);
$e->set('path_info', 'category/42/page/3'); // dummy pathinfo

$req = new WebRequest;
$e->registerFunction(array($req,'setRoute'), 'setRoute');

$e->run($router); // let js do the work

print_r($req);


