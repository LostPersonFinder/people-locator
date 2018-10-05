<?
$conf['approot'] = getcwd()."/../";
?>
<style>
  html, body { margin: 0; }
  .a {
    padding: 0.5em 1em;
    background: #2196F3;
    color: white;
    text-align: center;
    font-size: 1.5em;
  }
  .b { padding: 1em; }
  .c { width: 100%;  }
  .d { background-color: #1976D2; color: white; }
  td { padding:0 1em 0 1em; }
  tr:nth-child(even){ background-color: #eee; }
  table{ border-collapse:collapse; }
  * { font-family:monospace; font-size:14px; }
</style>
<div class="a">DIFF Comparison Tool</div>
<?
if(!isset($_GET['locale'])) {
  echo '<div class="b">Select a locale:<br>';
  foreach(glob('../www/assets/locales/*.json') as $filename) {
    $e = explode('/', $filename);
    $localeFile = $e[sizeof($e)-1];
    if($localeFile != 'en.json') { echo '<a href="?locale='.$localeFile.'">'.$localeFile.'</a><br>'; }
  }
  echo '</div>';
  die();
}
$obj = json_decode(file_get_contents('https://raw.githubusercontent.com/LostPersonFinder/taupo-i18n/master/en.json'));
foreach($obj as $key => $value) { $i18n['en.json'][$key] = $value->message; }
$obj = json_decode(file_get_contents('https://raw.githubusercontent.com/LostPersonFinder/taupo-i18n/master/'.$_GET['locale']));
foreach($obj as $key => $value) { $i18n[$_GET['locale']][$key] = $value->message; }
$count = 0;
echo '<table class="c"><td class="d">key</td><td class="d">message</td></tr>';
foreach($i18n[$_GET['locale']] as $a => $b) {
  if(isset($i18n['en.json'][$a]) && ($b == $i18n['en.json'][$a]) && $b != 'No') {
    echo '<tr><td>'.$a.'</td><td>'.$b.'</td></tr>'; 
    $count++;
  }
}
if($count == 0) { echo '<tr><td colspan=2>100%</td></tr>'; }
echo '</table>';
