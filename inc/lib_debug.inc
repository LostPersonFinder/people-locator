<?
/**
 * @name     debug lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */
 
 // log dao error
function daoErrorLog($file, $line, $method, $class, $function, $errorMessage, $other) {
  global $global;
  $db = $global['db'];
  // add backtrace
  $other .= " <+> ".debug_string_backtrace()." <+> ";
  $q = "
    INSERT INTO dao_error_log (
      file,
      line,
      method,
      class,
      function,
      error_message,
      other )
    VALUES (
      ".$global['db']->qstr((string)$file).",
      ".$global['db']->qstr((string)$line).",
      ".$global['db']->qstr((string)$method).",
      ".$global['db']->qstr((string)$class).",
      ".$global['db']->qstr((string)$function).",
      ".$global['db']->qstr((string)$errorMessage).",
      ".$global['db']->qstr((string)$other)." );
  ";
  $result = $db->Execute($q);
}

// produce back trace
function debug_string_backtrace() { 
  ob_start();
  debug_print_backtrace();
  $trace = ob_get_contents();
  ob_end_clean();
  // remove first item from backtrace as it's this function
  $trace = preg_replace ('/^#0\s+' . __FUNCTION__ . "[^\n]*\n/", '', $trace, 1);
  // renumber backtrace items
  $trace = preg_replace_callback(
    '/^#(\d+)/m',
    function($m) {
      return '#'.($m[1] - 1);
    },
    $trace);
  return $trace;
}

// dump a var to error log
function var_dump_error_log($mixed = null) {
  ob_start();
  var_dump($mixed);
  $content = ob_get_contents();
  ob_end_clean();
  error_log($content);
}
