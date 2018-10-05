<?
/**
 * @name     Administration
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global, $conf;
$global['use_stream'] = true; // use lib_stream
taupo_xajax_init();
require_once($conf['approot'].'/mod/admin/xajax.inc');
taupo_xajax_registerFunction('admin_show_dashboard');
taupo_xajax_registerFunction('admin_clear_ban_list');
taupo_xajax_registerFunction('admin_show_users');
taupo_xajax_registerFunction('admin_edit_user');
taupo_xajax_registerFunction('admin_save_edit');
taupo_xajax_registerFunction('admin_del_user_do');

// the controller for admin modole
function taupo_admin_default() {
  global $global, $conf;
  taupo_xajax_printJavascript();
  if(isset($_GET['mods']))      { $control = "taupo_admin_mods"; }  // /mods
  elseif(isset($_GET['users'])) { $control = "taupo_admin_users"; } // /users
  elseif(isset($_GET['log']))   { $control = "taupo_admin_log"; }   // log
  else                          { $control = "taupo_admin_dash"; }  // default
  $control();
}

function taupo_admin_dash() {
  global $global, $conf;
  $interval = 15000; // 15 seconds
  $time_file = $conf['approot']."time";
  if(file_exists($time_file)) {
    $handle2 = fopen($time_file, "r");
    $time = fgets($handle2);
  } else { $time = '00:00'; }
  echo '<div id="adminBOX">';
  echo '<script>'.file_get_contents($conf['approot'].'www/assets/bower_components/highcharts/highcharts.js').'</script>';
  echo '<script>'.file_get_contents($conf['approot'].'www/assets/bower_components/highcharts/modules/exporting.js').'</script>';  
  echo '<script>'.file_get_contents($conf['approot'].'mod/admin/dash.js').'</script>';
?>
  <div id="pc01">
    <div id="pingChart1" class="fadeToAndFromZero1 dashBox">
      <center>
        Loading... &nbsp;
      </center>
    </div>
    <div id="pingChart0" class="fadeToAndFromZero2 dashBox"></div>
  </div>
  <div style="clear: both;" class="fadeFromZero3">
    <div id="dashBoxes">
      <div class="dashBox">
        <center><paper-button class="nonUpper bold">Statistics</paper-button></center>
        <div id="statsDiv" style="text-align: left;"></div>
      </div>
      <div class="dashBox">
        <center><paper-button class="nonUpper bold hover" onclick="window.location='arrivals';">Arrivals</paper-button></center>
        <div id="arrivalsDiv" style="text-align: left;"></div>
      </div>
      <div class="dashBox">
        <div id="fmDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <div id="restDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <center><paper-button class="nonUpper bold hover" onclick="window.location='notify';">Push Devices</paper-button></center>
        <div id="notifyDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <center><paper-button class="nonUpper bold">Banned</paper-button></center>
        <div id="ipDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <div id="emailDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <div id="abuseDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <div id="solrDiv" class="mainArea"></div>
      </div>
      <div class="dashBox">
        <div id="pfifDiv" class="mainArea"></div>
      </div>
      <div style="clear:both"></div>
    </div>
    <script>
      // main()
      $(document).ready(function() {
        goPingChart();
        admin_show_dashboard();
        setTimeout(function() {
          $('#pingChart1').animate({ opacity: 0 }, 2000);
          $('#pingChart0').animate({ opacity: 1 }, 4000);
        }, 4000);
        setInterval('admin_show_dashboard()', <? echo (int)$interval; ?>);
      });
    </script>
  </div>
</div><!-- adminBOX -->
<div id="allUpdated"> &nbsp; </div>
<div id="build">Taupo Build Time: <? echo $time; ?> // <a id="modacl" href="mods">Modules</a></div>
<?
}

/*
<h2><sup>*</sup>Ping Times</h2>
<li>This is not a true ping test (a real ping is UDP+ICMP and here we use TCP+HTTP).</li>
<li>The data points displayed here are the elapsed times during round trip jQuery $.get XHR requests (once a second).</li>
<li>Each request is downloading a file of 1-byte size from the server (the url of this file has a random query string appended to avoid caching).</li>
<li>We begin timing when the request is made and stop when the file is successfully retrieved.</li>
<li>It has been observed that the times here are on average 10-12x larger than actual ICMP ping times.</li>
<li>Three factors can contribute to large ping times here: 1) high server load 2) network latency 3) high client (browser) load.</li>
<div style="clear: both;"></div>
*/

function taupo_admin_mods() {
  global $global, $conf;
  $perms = "";
  echo '<div id="adminHolder" class="biuHolder">';
  echo '
    <center>Module Access Control List<br>
    <table style="width: 100%;">
      <thead>
        <td><b>Module</b></td>
        <td><b>Admin</b></td>
        <td><b>User</b></td>
        <td><b>Anon</b></td>
      </thead>
      <tbody>
  ';
  foreach($conf['acl'] as $mod=>$acl) {
    echo '<tr class="modhover"><td>'.$conf['mod_'.$mod.'_name'].'</td>';
    if(in_array(1, $acl)) { echo '<td>X</td>'; }
    else                  { echo '<td>&nbsp;</td>'; }
    if(in_array(2, $acl)) { echo '<td>X</td>'; }
    else                  { echo '<td>&nbsp;</td>'; }
    if(in_array(3, $acl)) { echo '<td>X</td>'; }
    else                  { echo '<td>&nbsp;</td>'; }
    echo '</tr>';
  }
  echo '</tbody></table></div>';
}

function taupo_admin_users() {
  global $global, $conf;
  echo '<script>'.file_get_contents($conf['approot']."/mod/admin/users.js").'</script><div id="adminBOX">';
?>
  <div id="rollup">
    <div class="userBg">
      <div id="searchOptions">
        <div id="userSearch">
          <paper-input no-label-float name="userSearchBox" id="userSearchBox" label="search by user" value=""></paper-input>
        </div>
        <paper-radio-group class="left" id="selector" selected="0">
          <paper-radio-button class="left" id="showAll"     name="0">ALL</paper-radio-button>
          <paper-radio-button class="left" id="showPending" name="1">Pending</paper-radio-button>
          <paper-radio-button class="left" id="showInactive"name="3">Inactive</paper-radio-button>
          <paper-radio-button class="left" id="showAdmin"   name="2">Admin</paper-radio-button>
        </paper-radio-group>
        <paper-checkbox id="limiter" class="left" label="limit" checked role="checkbox" checked>Limit 100</paper-checkbox>
      </div>
      <div id="addNewUserDiv">
        <paper-button raised="" class="buttonEM bgYellow addNewUserButton" onclick="editUser(9999,9999);">Add New User</paper-button>
      </div>
    </div>
    <div id="usersDiv" style="clear: both; padding-top: 0.25em;" class="mainArea"></div>
  </div>
  <div id="editOneUser"></div>
  <script>
    $(function() { $("#showAll"     ).click(function(e) { showAll();       }); });
    $(function() { $("#showPending" ).click(function(e) { showPending();   }); });
    $(function() { $("#showInactive").click(function(e) { showInactive();  }); });
    $(function() { $("#showAdmin"   ).click(function(e) { showAdmin();     }); });
    $(function() { $("#limiter"     ).change(function(e) { refreshUsers(); }); });
    admin_show_users();
    $("#userSearch").on("change keyup paste", function(){ refreshUsers(); });
  </script>
  </div>
  <paper-dialog id="deleteUserOk" class="none" modal entry-animation="scale-up-animation" exit-animation="fade-out-animation">
    <h2>DELETE USER CONFIRMATION</h2>
    <paper-dialog-scrollable>
      Are you sure you want to delete this user? This action cannot be undone and is audited.
    </paper-dialog-scrollable>
    <div class="buttons">
      <paper-button dialog-dismiss class="hover" onclick="console.log('delete-user-decline');">Cancel</paper-button>
      <paper-button dialog-confirm class="hover" onclick="delUser2();">Accept</paper-button>
    </div>
  </paper-dialog>
<?
}

function taupo_admin_log() {
  global $global, $conf;
  echo '<div id="adminBOX" class="four44">';
  $count = 0;
  if(!isset($_GET['page'])) { $_GET['page'] = 1; }
  $sql = "
    SELECT count(*)
    FROM `audit`
  ";
  $res = $global['db']->Execute($sql);
  while($row = $res->FetchRow() ) { $numb = $row["count(*)"]; }
  $perPageLimit = 100;
  $totalPagesLimit = 50;
  if(($numb/$perPageLimit) < $totalPagesLimit) {
    $perPage = $perPageLimit;
    $totalPages = ceil($numb / $perPageLimit);
  } else {
    $perPage = ceil($numb/$totalPagesLimit) ;
    $totalPages = $totalPagesLimit;
  }
  // sort ip asc
  if(isset($_GET['SORT5'])) {
    $sort = 'ORDER BY `ip` ASC';
    $sorte = 'SORT5';
  // sort ip desc
  } elseif(isset($_GET['SORT4'])) {
    $sort = 'ORDER BY `ip` DESC';
    $sorte = 'SORT4';
  // sort type asc
  } elseif(isset($_GET['SORT3'])) {
    $sort = 'ORDER BY `type` ASC';
    $sorte = 'SORT3';
  // sort type desc
  } elseif(isset($_GET['SORT2'])) {
    $sort = 'ORDER BY `type` DESC';
    $sorte = 'SORT2';
  // sort time asc
  } elseif(isset($_GET['SORT1'])) {
    $sort = 'ORDER BY `time` ASC';
    $sorte = 'SORT1';
  // sort time desc
  } else {
    $sort = 'ORDER BY `time` DESC';
    $sorte = 'SORT0';
  }
  $pagey = '<div class="pagey">Page: ';
  for($i = 1; $i <= $totalPages; $i++) {
    if($i == $_GET['page']) { $pagey .= '<span style="font-weight: bold;">'.$i.'</span> ';  }
    else                    { $pagey .= '<a href="log?page='.$i.'&'.$sorte.'">'.$i."</a> "; }
  }
  $pagey .= '</div>';
  echo $pagey;
  echo '
    <table class="auditable">
    <thead>
      <td class="center">
        Time &nbsp; 
        <a href="log?page='.$_GET['page'].'&SORT0">▼</a>
        <a href="log?page='.$_GET['page'].'&SORT1">▲</a>
      </td>
      <td class="center">
        Type &nbsp; 
        <a href="log?page='.$_GET['page'].'&SORT2">▼</a>
        <a href="admin?audit&tabid=6&page='.$_GET['page'].'&SORT3">▲</a>
      </td>
      <td class="center">
        IP &nbsp; 
        <a href="log?page='.$_GET['page'].'&SORT4">▼</a>
        <a href="log?page='.$_GET['page'].'&SORT5">▲</a>
      </td>
      <td>Notes</td>
    </thead>
    <tbody>
  ';
  $limitStart = isset($_GET['page']) ? ($_GET['page'] - 1)*$perPage : 1;
  $limitEnd   = $limitStart + $perPage - 1;
  $sql = "
    SELECT *
    FROM `audit`
    ".$sort."
    LIMIT ".$limitStart.", ".$limitEnd.";
  ";
  $res = $global['db']->Execute($sql);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "acl event log display (".$sql.")"); }
  while(($res != null) && (!$res->EOF)) {
    $timestamp = $res->fields["time"];
    $type = ($res->fields["type"] == null) ? "" : $res->fields["type"];
    $ip = ($res->fields["ip"] == null) ? "" : $res->fields["ip"];
    echo '
    <tr>
      <td class="center" style="min-width: 10em;">'.$timestamp.'</td>
      <td class="center" style="min-width: 14em; color: red; font-weight: bold;">'.$type.'</td>
      <td class="center" style="padding: 0 1em;">'.$ip.'</td>
      <td>'.$res->fields['notes'].'</td>
    </tr>
    ';
    $res->MoveNext();
    $count++;
  }
  if($count == 0) { echo "<tr><td colspan=4><center>No log entries thus far.</center></td></tr>"; }
  echo '</tbody></table>';
  echo $pagey;
  echo '</div>';
}
