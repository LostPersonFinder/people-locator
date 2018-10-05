<?
/**
 * @name     stream lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function stream() {
  global $global, $conf;
  // stream ajax
  if(array_key_exists('stream', $_GET) && $_GET['stream'] === 'ajax') {
    taupo_xajax_init();
    $module_function = 'taupo_ajax_'.$global['module'].'_'.$global['action'];
    // validate xajax module permission
    $pieces = explode('_', $_POST['xajax']);
    $xmod = $pieces[0];
    if(!in_array($_SESSION['gid'], $conf['acl'][$xmod]) || !function_exists($module_function)) {
      http_response_code(404);
      die(); // illegal xajax request
    }
    // do ajax
    taupo_ajax_start();
    $module_function();
    taupo_ajax_end();
  // stream html
  } else {
    $module_function = 'taupo_'.$global['module'].'_'.$global['action'];
    // validate function
    if(!function_exists($module_function)) {
      $global['module'] = $conf['default_mod'];
      $global['action'] = 'default';
      $global['toast']  = 'moduleInvalid';
      $module_function  = 'taupo_'.$global['module'].'_'.$global['action'];
      http_response_code(404);
    }
    // do html
    taupo_html_start();
    $module_function();
    taupo_html_end();
  }
}

// html stream start
function taupo_html_start() {
  global $conf, $global;
  // a module may bypass the html stream
  if(!isset($global['use_stream'])) { return; }
  // load and compress css
  $css = (string)file_get_contents($conf['approot']."inc/styles.css");
  // headers
  ob_start("ob_gzhandler"); // enable gzip
  header("Content-Type: text/html; charset=utf-8");
  header("Cache-Control: no-store, must-revalidate, max-age=0");
  header("Expires: ".date('D, j M Y H:i:s')." GMT", time()+(60*60)); // expire in 1 hour "Expires: Sat, 26 Jul 1997 05:00:00 GMT"
  header("X-UA-Compatible: IE=edge"); // force latest edge rendering engine
  // page
?><!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge" />
  <meta name="viewport" content="width=device-width, minimum-scale=1.0, initial-scale=1, user-scalable=yes">
  <meta name="theme-color" content="#000000">
  <link href="//fonts.googleapis.com/css?family=Open+Sans:300italic,300,400italic,400,600italic,600,700italic,700,800italic,800" rel="stylesheet" type="text/css">
  <script src="assets/bower_components/webcomponentsjs/webcomponents-lite.min.js"></script>
  <script src="assets/bower_components/jquery/dist/jquery.min.js"></script>
  <link rel="import" href="assets/bower_components/paper-button/paper-button.html">
  <link rel="import" href="assets/bower_components/paper-checkbox/paper-checkbox.html">
  <link rel="import" href="assets/bower_components/paper-dialog/paper-dialog.html">
  <link rel="import" href="assets/bower_components/paper-input/paper-input.html">
  <link rel="import" href="assets/bower_components/paper-input/paper-input-char-counter.html">
  <link rel="import" href="assets/bower_components/paper-item/paper-item.html">
  <link rel="import" href="assets/bower_components/paper-radio-group/paper-radio-group.html">
  <link rel="import" href="assets/bower_components/paper-ripple/paper-ripple.html">
  <link rel="import" href="assets/bower_components/paper-slider/paper-slider.html">
  <link rel="import" href="assets/bower_components/paper-tabs/paper-tabs.html">
  <link rel="import" href="assets/bower_components/paper-toast/paper-toast.html">
  <link rel="import" href="assets/bower_components/paper-toggle-button/paper-toggle-button.html">
  <link rel="import" href="assets/bower_components/iron-icons/iron-icons.html">
  <link rel="import" href="assets/bower_components/neon-animation/animations/scale-up-animation.html">
  <link rel="import" href="assets/bower_components/neon-animation/animations/fade-out-animation.html">
  <title>ADMIN</title>
  <? echo "<style>".$css."</style>\n"; ?>
</head>
<body>
  <div id="container">
    <div id="sitehead">
      <div id="site_name" class="clickable linkHome"><a href="<? echo $conf['base_url']; ?>">PEOPLE LOCATOR</a> // <a href="admin">ADMIN</a></div>
      <div id="user_info">user <span style="color: orange;">//</span> <? echo $_SESSION['user']; ?></div>
    </div>
    <div id="menu" class="both">
      <a href="admin">Dashboard</a> // 
      <a href="events">Event Manager</a> //
      <a href="users">Users</a> //
      <a href="upload">Batch Upload</a> // 
      <a href="batch">Batch Report</a> //
      <a href="log">Log</a>
    </div><?
}

// html stream end
function taupo_html_end() {
  global $conf, $global;
  if(!isset($global['use_stream'])) { return; }
  ?>
    <paper-toast id="toast1" style="color: white !important; background-color: red;"    duration="10000"></paper-toast>
    <paper-toast id="toast2" style="color: black !important; background-color: yellow;" duration="10000"></paper-toast>
    <paper-toast id="toast3" style="color: white !important; background-color: green;"  duration="10000"></paper-toast>
    <script>
    // show toast 1
    function toast(msg) {
      document.querySelector('#toast1 #label').innerHTML = msg;
      document.querySelector('#toast1').show();
    }
    // show toast 2
    function toast2(msg) {
      document.querySelector('#toast2 #label').innerHTML = msg;
      document.querySelector('#toast2').show();
    }
    // show toast 3
    function toast3(msg) {
      document.querySelector('#toast3 #label').innerHTML = msg;
      document.querySelector('#toast3').show();
    }
  </script>
  </div><!-- containter -->
</body>
</html><?
  echo "<script>window.module = '".$global['module']."';</script>";
  ob_end_flush();
}
