<?
/**
 * @name     widget
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// main module function ~ render widget help page
function taupo_widget_default() {
  global $global, $conf;
  taupo_widget_main();
  echo '<div id="wighelper">';
  echo '<iframe title="widget with no default event" src="'.$conf['base_url'].'/widgeti'.'" width=170 height=225 frameborder=0 scrolling="no"></iframe><br><br>';
  echo 'To embed the above widget on your site with no default event selected, simply add the following code:<br><br>';
  echo '<div class="coder"><b>&lt;iframe src="'.$conf['base_url'].'/widgeti'.'" width=170 height=225 frameborder=0 scrolling="no"&gt;&lt;/iframe&gt;</b></div><br><br><br>';
  echo '<iframe title="widget with default event" src="'.$conf['base_url'].'/widgeti?e=global'.'" width=170 height=225 frameborder=0 scrolling="no"></iframe><br><br>';
  echo 'However, if you would like to have the widget load with a default event selected (as above), use this code instead:<br><br>';
  echo '<div class="coder"><b>&lt;iframe src="'.$conf['base_url'].'/widgeti?e=EVENTSHORTNAME" width=170 height=225 frameborder=0 scrolling="no"&gt;&lt;/iframe&gt;</b></div><br>';
  echo 'You should replace EVENTSHORTNAME with the event of your choice from the list of current events below:<br><br>';
  echo '<table><tr><td><i>Event Name</i></td><td><i>Event Short Name</i></td></tr>';
  $q = "
    SELECT *
    FROM incident
    WHERE `private_group` IS NULL
    AND unlisted = 0;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "widget 1"); }
  while (!$result == NULL && !$result->EOF) {
    echo '<tr><td>'.json_decode($result->fields["translations"])->en->name.'</td><td><b>'.$result->fields["shortname"].'</b></td></tr>';
    $result->MoveNext();
  }
  echo '</table><br><br><br>';
  echo '<iframe title="widget with non-English locale" src="'.$conf['base_url'].'/widgeti?l=es'.'" width=170 height=225 frameborder=0 scrolling="no"></iframe><br><br>';
  echo 'Finally, if you would like to have the widget use a non-English locale (as above), use this code instead:<br><br>';
  echo '<div class="coder"><b>&lt;iframe src="'.$conf['base_url'].'/widgeti?l=LOCALE" width=170 height=225 frameborder=0 scrolling="no"&gt;&lt;/iframe&gt;</b></div><br>';
  echo 'You should replace LOCALE with the locale of your choice from the list of currently supported locales below:<br><br>';
  echo '<table><tr><td><i>Language</i></td><td><i>Locale</i></td></tr>';
  echo '<tr><td>Spanish</td><td>es</td></tr>';
  echo '<tr><td>Japanese</td><td>ja</td></tr>';
  echo '<tr><td>Urdu</td><td>ur</td></tr>';
  echo '<tr><td>Vietnamese</td><td>vi</td></tr>';
  echo '<tr><td>Simplified Chinese</td><td>zh_CN</td></tr>';
  echo '<tr><td>Traditional Chinese</td><td>zh_TW</td></tr>';
  echo '</table></div>';
  echo "<script>ga('send','pageview','/widget');</script>";
  echo '</body></html>';
}

// basic server response
function taupo_widget_main() {
  global $global, $conf;
  // load and compress css
  $buffer = file_get_contents($conf['approot']."mod/widget/styles.css");                     // load styles 1
  $buffer = preg_replace('!/\*[^*]*\*+([^/][^*]*\*+)*/!', '', $buffer);                      // Remove comments
  $buffer = str_replace(': ', ':', $buffer);                                                 // Remove space after colons
  $buffer = str_replace(array("\r\n", "\r", "\n", "\t", '  ', '    ', '    '), '', $buffer); // Remove whitespace
  // headers
  ob_start("ob_gzhandler"); // Enable gzip compression
  header("Content-Type: text/html; charset=utf-8");
  header("Cache-Control: no-store, must-revalidate, max-age=0");
  header("Expires: ".date('D, j M Y H:i:s')." GMT", time()+(60*60*1)); // expires in 1 hour "Expires: Sat, 26 Jul 1997 05:00:00 GMT"
  header("X-UA-Compatible: IE=edge"); // force latest IE; PL-1030
  // page
  echo '<!doctype html>';
  echo '<html lang="en" style="background-color: rgb(233, 233, 233);">';
  echo '<head>';
  echo '<meta http-equiv="X-UA-Compatible" content="IE=edge" />';
  echo '<meta name="viewport" content="width=480, initial-scale=1.0">';
  echo '<meta name="mobile-web-app-capable" content="yes">';
  echo '<meta name="apple-mobile-web-app-capable" content="yes">';
  echo '<link rel="chrome-webstore-item" href="https://chrome.google.com/webstore/detail/fbnpmpdcnjkhfcgeeklebjmopaheplce"/>';
  echo '<link href="//fonts.googleapis.com/css?family=Open+Sans:300italic,300,400italic,400,600italic,600,700italic,700,800italic,800" rel="stylesheet" type="text/css">';
  echo '<title>'.$conf['site_name'].'</title>';
  echo '<link rel="icon" type="image/png" sizes="32x32" href="/assets/icons/favicon-32x32.png">';
  echo '<link rel="icon" type="image/png" sizes="16x16" href="/assets/icons/favicon-16x16.png">';
  echo '<link rel="apple-touch-icon" sizes="180x180" href="/assets/icons/apple-touch-icon.png">';
  echo '<link rel="mask-icon" href="/assets/icons/safari-pinned-tab.svg" color="#5bbad5">';
  echo '<script src="assets/bower_components/jquery/dist/jquery.min.js"></script>';
  echo '<style>'.$buffer.'</style>';
  echo '<script>'.file_get_contents($conf['approot']."mod/widget/main.min.js").'</script>';
  echo "<script>window.ga=window.ga||function(){(ga.q=ga.q||[]).push(arguments)};ga.l=+new Date;ga('create', 'UA-49715038-1', 'auto');</script>";
  echo "<script async src='https://www.google-analytics.com/analytics.js'></script>";
  echo '</head>';
  echo '<body>';
}

// render widget
function taupo_widget_inc() {
  global $global, $conf;
  taupo_widget_main();
  echo '<div class="bodyDiv">';
  $e = isset($_REQUEST['e']) ? $_REQUEST['e'] : null;
  $locale = isset($_REQUEST['l']) ? $_REQUEST['l'] : "en";
  $q = isset($_POST['query']) ? $_POST['query']        : '';
  // get language strings
  $obj = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$locale.'.json'));
  $selectEventM = $obj->selectEvent->message;
  $enterNameM   = $obj->enterName->message;
  $foundM       = $obj->statusFound->message;
  $recordsM     = $obj->records->message;
  $searchM      = $obj->searchPlaceholder->message;
  $reportM      = $obj->tabReport->message;
  // if event, then search
  if($e != null) {
    $error      = false;
    $shortname  = $e;
    $query      = $q;
    $searchMode = $conf['SOLR_on'] == true ? "solr" : "sql";
    // search
    $search = new SearchDB($searchMode, $shortname, $query);
    try { $search->executeSearch(); }
    catch (Exception $e) { $error = true; }
    if($error) { $resultString = ''; }
    else {
      $count = (int)$search->numRowsFound;
      $resultString = '
        <a href="'.$conf['base_url'].'/en/'.$e."/".$q.'" target="_blank">
        <span class="count">'.$foundM.' '.$count.' '.$recordsM.'</span></a>
        <input type="button" value="View the Results" onclick="window.open(\''.$conf['base_url'].'/'.$locale.'/'.$e.'/'.$q.'\');" class="button z9"/>
      ';
    }
  } else { $resultString = ''; }
  ?>
    <form id="searchForm" method="post">
      <div class="header">
        <h1>
          <a href="<? $conf['base_url'] ?>" target="_blank">PEOPLE LOCATOR<Font SIZE = "1" ><B style="vertical-align:top">&#174;</B></FONT></a>
        </h1>
        <select name="e" id="e" style="width: 135px;" onchange="this.form.submit()" aria-label="<? echo $selectEventM ?>">
          <option value=""><? echo $selectEventM ?></option>
  <?
  $qq = "
    SELECT *
    FROM incident
    WHERE `private_group` IS NULL
    AND unlisted = 0;
  ";
  $result = $global['db']->Execute($qq);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "widget 2"); }
  while (!$result == NULL && !$result->EOF) {
    echo '<option value="'.$result->fields["shortname"].'" ';
    if($result->fields["shortname"] == $e) { echo 'selected'; }
    echo '>'.json_decode($result->fields["translations"])->en->name.'</option>';
    $result->MoveNext();
  }
  ?>
        </select>
      </div>
      <div class="container">
        <div class="search">
          <div id="result" class="subHeader"><? echo $resultString ?></div>
          <input class="textInput" type="text" autocomplete="off" placeholder="<? echo $enterNameM ?>" name="query" id="query" size="25" value="<? echo $q; ?>" aria-label="<? echo $enterNameM ?>" title="<? echo $enterNameM ?>" />
          <input id="search" type="submit" value="<? echo $searchM ?>" class="button" onclick="return validate();" padding-left="10px"/>
          <input id="report" type="button" value="<? echo $reportM ?>" class="button" onclick='validate() && window.open("<? echo $conf['base_url'].'/'.$locale.'/'.$s.'/report'; ?>")'/>
          <div style="margin-top:10px;color:white;font-size:x-small;">
            <a href="http://www.nlm.nih.gov/" target="_blank" title="go to NLM Home Page">U.S. National Library of Medicine</a>
          </div>
        </div>
        <div class="bgImage"></div>
      </div>
    </form>
  </div>
  <!-- Only record pageviews for widgets on other domains. --> 
  <script>try {var testdomain = window.parent;} catch(e) {ga('send','pageview',window.location.pathname);}</script>
</body>
</html>
<?
}
