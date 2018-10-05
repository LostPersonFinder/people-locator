<? ?><!-- // show i18n strings in table -->
<html><style>
html, body { margin: 0; padding: 0; }
body { width: 100%; max-width: 100%; }
.top { background-color: green; color: white; text-align: center; }
.top:hover { background-color: green; }
.even { background-color: #eee; }
.odd { background-color: #ddd; }
.bold {font-weight: bold; }
tr:hover { background-color: yellow; }
table { border-spacing: 0; border-collapse: separate; width: 100%; max-width: 100%; }
td { padding: 0.5em 0; font-size: 14px;}
.tdtop { padding: 1em; }
.center { text-align: center !important; }
.left { text-align: left !important; }
.same { color: rgb(255, 0, 243); }
</style><body><?
foreach(glob('../www/assets/locales/*.json') as $filename) {
  $obj = json_decode(file_get_contents($filename));
  $e = explode('/', $filename);
  $localeFile = $e[sizeof($e)-1];
  foreach($obj as $key => $value) { $i18n[$key][$localeFile] = $value->message; }
}
echo '<table><tr class="top"><td class="tdtop left"># String ID / Locale File</td>';
foreach($i18n['thisLanguage'] as $key => $value) { echo '<td class="left">'.$key.'</td>'; }
echo '</tr>';
$eo = 0;
$count = 1;
foreach($i18n as $key => $value) {
  if($eo == 0) { $eoc = 'even'; }
  else { $eoc = 'odd'; }
  echo '<tr class="'.$eoc.'"><td class="center left">'.$count.' <span class="bold">'.$key.'</span></td>';
  foreach($value as $a => $b) {
    $same = '';
    if(isset($value['en.json']) && $b == $value['en.json']) { $same = 'same'; }
    echo '<td class="'.$same.'">'.$b.'</td>';
  }
  echo '</tr>';
  $eo++;
  if($eo > 1) { $eo = 0; }
  $count++;
} ?></table></body></html>
