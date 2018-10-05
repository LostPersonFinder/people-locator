<?
require_once('/home/g/taupo.production/conf/config_PL.inc');
require_once('/home/g/taupo.production/inc/lib_includes.inc');
global $i18n;
$pi18n = $conf['i18n_on'];
require_once('../conf/config_PL.inc');
$out = '<style>.center{text-align: center;}td{padding:0 1em 0 0.5em;}tr:nth-child(odd){background-color: #eee;}table{border-collapse:collapse;}*{font-family:monospace;font-size:14px;}a{text-decoration:none;line-height:22px;}</style>';
foreach(glob('../www/assets/locales/*.json') as $filename) {
  // staging
  $obj = json_decode(file_get_contents($filename));
  $e = explode('/', $filename);
  $localeFile = $e[sizeof($e)-1];
  foreach($obj as $key => $value) { $i18n[$localeFile][$key] = $value->message; }
  // production
  $prod = '/home/g/taupo.production/www/assets/locales/';
  $obj  = json_decode(file_get_contents($prod.$localeFile));
  foreach($obj as $key => $value) { $i18n[$localeFile.'_pro'][$key] = $value->message; }
  // github
  $obj = json_decode(file_get_contents('https://raw.githubusercontent.com/LostPersonFinder/taupo-i18n/master/'.$localeFile));
  foreach($obj as $key => $value) { $i18n[$localeFile.'_hub'][$key] = $value->message; }
}
$r = [];
foreach($i18n as $key => $value) { $r[$key] = 0; }
$count     = 0;
$count_hub = 0;
$count_pro = 0;
foreach($i18n as $key => $value) {
  foreach($value as $a => $b) {
         if(substr($key,-4) == '_hub') { if(isset($i18n['en.json_hub'][$a]) && ($i18n['en.json_hub'][$a] == 'No' || ($b != $i18n['en.json_hub'][$a]))) { $r[$key]++; }}
     elseif(substr($key,-4) == '_pro') { if(isset($i18n['en.json_pro'][$a]) && ($i18n['en.json_pro'][$a] == 'No' || ($b != $i18n['en.json_pro'][$a]))) { $r[$key]++; }}
    else {                               if(isset($i18n['en.json'][$a])     && ($i18n['en.json'][$a]     == 'No' || ($b != $i18n['en.json'][$a])))     { $r[$key]++; }}
    if($key == 'en.json')     { $count++;}
    if($key == 'en.json_hub') { $count_hub++; }
    if($key == 'en.json_pro') { $count_pro++; }
  }
}
$r['en.json']     = $count;
$r['en.json_hub'] = $count_hub;
$r['en.json_pro'] = $count_pro;
$out .= '<tr style="font-weight: bold;"><td>Enabled Locales</td><td colspan=2>Github</td><td colspan=2>Staging/PLStage</td><td colspan=2>Production/PL</td></tr>';
foreach($conf['i18n_on'] as $on) {
  $huh = in_array($on, $pi18n);
  if($huh){ $showp = '<td>'.(int)($r[$on.'.json_pro']/$count_pro *100).'%</td><td>('.str_pad($r[$on.'.json_pro'],3,'_',STR_PAD_LEFT).'/'.$count_pro.')'.'</td>'; }
  else { $showp = '<td colspan=2><span style="color: green">Coming Soon !!!</span></td>'; }
  $out .= '<tr>';
  $out .= '<td>'.$i18n[$on.'.json']['thisLanguageDescription'].'</td>';
  $out .= '<td>'.(int)($r[$on.'.json_hub']/$count_hub*100).'%</td><td>('.str_pad($r[$on.'.json_hub'],3,'_',STR_PAD_LEFT).'/'.$count_hub.')</td>';
  $out .= '<td>'.(int)($r[$on.'.json']    /$count    *100).'%</td><td>('.str_pad($r[$on.'.json'],    3,'_',STR_PAD_LEFT).'/'.$count    .')</td>';
  $out .= $showp;
  $out .= '</tr>';
}
$out .= '<tr><td colspan=7>&nbsp;</td></tr><tr style="font-weight: bold;"><td>Disabled Locales</td><td colspan=2>Github</td><td colspan=2>Staging/PLStage</td><td colspan=2>Production/PL</td></tr>';
foreach($conf['i18n_off'] as $on) {
  $out .= '<tr>';
  $out .= '<td>'.$i18n[$on.'.json']['thisLanguageDescription'].'</td>';
  $out .= '<td>'.(int)($r[$on.'.json_hub']/$count_hub*100).'%</td><td>('.str_pad($r[$on.'.json_hub'],3,'_',STR_PAD_LEFT).'/'.$count_hub.')</td>';
  $out .= '<td>'.(int)($r[$on.'.json']    /$count    *100).'%</td><td>('.str_pad($r[$on.'.json'],    3,'_',STR_PAD_LEFT).'/'.$count    .')</td>';
  $out .= '<td>'.(int)($r[$on.'.json_pro']/$count_pro*100).'%</td><td>('.str_pad($r[$on.'.json_pro'],3,'_',STR_PAD_LEFT).'/'.$count_pro.')</td>';
  $out .= '</tr>';
}
$out = '<table>'.str_replace('_','&nbsp;',$out).'</table>';
$out .= '<br><a href="https://ceb-stage.nlm.nih.gov/~g/taupo/tools/i18n-summary.php">Link to this summary</a><br>';
$out .= '<a href="https://github.com/LostPersonFinder/taupo-i18n">Github for Translators</a><br>';
$out .= '<a href="https://ceb-stage.nlm.nih.gov/~g/taupo/tools/i18n-diff.php">DIFF Comparison Tool for finding missing strings</a><br>';
$out .= '<a href="https://ceb-stage.nlm.nih.gov/~g/taupo/tools/i18n-table-out.php">Side by Side of staging strings</a><br>';
$out .= '<a href="https://ceb-stage.nlm.nih.gov/~g/taupo/tools/i18n-old-key-search.php">Locate orphaned strings</a><br>';
$out .= '<a href="https://wiki.nlm.nih.gov/confluence/x/h4CuBg">PL Development & Testing Cycle</a>';
// key required to send email
if(isset($_REQUEST['key']) && $_REQUEST['key'] == $conf['service_key']) {
  $p = new email();
  $p->smtp_reply_address = 'i18n@plstage.nlm.nih.gov';
  $subject = 'PL Internationalization Weekly Status Report';
  $body = $out;
  $p->sendMessage('NLMLHCCEBLPF@mail.nlm.nih.gov', $subject, $body, '', 'internationalization@plstage.nlm.nih.gov');
} else { echo $out; }
