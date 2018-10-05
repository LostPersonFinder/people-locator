<?
// search for and find old unused i18n strings by key

// currently strings are in these modules:
// mod/home4
// mod/rest
// mod/tally
// mod/widget

echo '<pre>';
$count = 0;
$fail  = 0;
$path  = getcwd()."/../";
$obj   = json_decode(file_get_contents($path.'www/assets/locales/en.json'));
foreach($obj as $key => $value) {
  $found1 = php_grep($key, $path.'mod', false); // search all of /mod folder recursively
  $found2 = php_grep($key, $path.'inc', false); // search all of /inc folder recursively
  $found = $found1 || $found2;
  if($found) { $pf = '✔️'; } else { $pf = '❌ "'.$value->message.'"'; $fail++; }
  echo $key.': '.$pf.'<br>';
  $count++;
}
echo '-----------------------------<br>TOTAL: '.$count.'<br>USED: '.($count-$fail).'<br>UNUSED: '.$fail;

function php_grep($key, $path, $found = false) {
  //echo 'searching path: '.$path.'<br>';
  $fp = opendir($path);
  while($f = readdir($fp)){
    if(preg_match("#^\.+$#", $f)) continue; // ignore symbolic links
    $file_full_path = $path.'/'.$f;
    //echo 'searching file: '.$file_full_path.'<br>';
    if(is_dir($file_full_path)) { $found = php_grep($key, $file_full_path, $found); }
    elseif(stristr(file_get_contents($file_full_path), $key)) {
      //echo '^---FOUND!<br>';
      $found = true;
    }
  }
  return $found;
}
