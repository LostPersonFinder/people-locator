<?
/**
 * @name     home4 build lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */
 
go();
 function go() {
   global $argv;
   if($argv[1] == 'mergeCss') { mergeCss(); }
   elseif($argv[1] == 'finalizeBuild') { finalizeBuild(); }
 }
 function mergeCss() {
    $dir = '/opt/pl/www/assets/app/';
    $a = file_get_contents($dir.'b.html');
    $a = str_replace('reCSSO', file_get_contents($dir.'a.css'), $a);
    file_put_contents($dir.'b.html', $a);
 }
 function finalizeBuild() {
    $dir = '/opt/pl/www/assets/app/';
    $a = file_get_contents($dir.'a.html');
    $a = str_replace('<script src="b.js" defer=""></script>', '<script defer src="/assets/app/a.js"></script>', $a);
    $a = str_replace('<script defer src=b.js></script>',      '<script defer src="/assets/app/a.js"></script>', $a);
    $a = str_replace('<script src=b.js defer></script>',      '<script defer src="/assets/app/a.js"></script>', $a);
    file_put_contents($dir.'a.html', $a);
 }
