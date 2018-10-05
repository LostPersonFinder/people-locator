<?
// Generate sitemap.xml with urls for all public, non-privileged events.
// We also specify alternate language pages here rather than as an html link element in page headers.
// See Deep Link Map on Wiki for comprehensive list of links included here.

global $global, $conf;

error_reporting(E_ALL);
ini_set("display_errors", "stderr");

$locales = $conf['i18n_on'];

// static pages
$statics = ['help', 'about', 'privacy', 'resources', 'follow', 'trademark', 'omb', 'widget'];

// get all event shortnames
$shortnames = [];
$q = "
  SELECT shortname
  FROM incident
  WHERE NOT unlisted
  AND shortname NOT LIKE '%test%'
  AND private_group IS NULL
";
$res = $global['db']->Execute($q);
if ($res === false) die ($global['db']->ErrorMsg());
while ($row = $res->FetchRow()) {
  $shortnames[] = $row['shortname'];
}

// header
$sXML = 
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
  "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\"\n".
  "xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"\n".
  "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n".
  "xsi:schemaLocation=\"http://www.sitemaps.org/schemas/sitemap/0.9\n".
  "http://www.sitemaps.org/schemas/sitemap/0.9/sitemap.xsd\">\n";
  
foreach ($locales as $locale) {
  // /en
  $sXML .= "  <url>\n";
  $sXML .= "    <loc>".$conf['base_url']."/".$locale."</loc>\n"; 
  foreach ($locales as $locale2) {
    $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'"/>'."\n";
  }
  $sXML .= '      <xhtml:link rel="alternate" hreflang="x-default" href="'.$conf['base_url'].'/en"/>'."\n";
  $sXML .= "  </url>\n";
  foreach ($statics as $static) {
    // /en/pages/help
    $sXML .= "  <url>\n";
    if ($static == 'widget') {  //widget page not localized
      $sXML .= "    <loc>".$conf['base_url']."/".$static."</loc>\n"; 
    } else {
      $sXML .= "    <loc>".$conf['base_url']."/".$locale."/pages/".$static."</loc>\n"; 
      foreach ($locales as $locale2) {
        $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'/pages/'.$static.'"/>'."\n";
      }
      $sXML .= '      <xhtml:link rel="alternate" hreflang="x-default" href="'.$conf['base_url'].'/en/pages/'.$static.'"/>'."\n";
    }
    $sXML .= "  </url>\n";
  }
  foreach ($shortnames as $shortname) {
    // /en/test
    $sXML .= "  <url>\n";
    $sXML .= "    <loc>".$conf['base_url']."/".$locale."/".$shortname."</loc>\n"; 
    foreach ($locales as $locale2) {
      $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'/'.$shortname.'"/>'."\n";
    }
    $sXML .= '      <xhtml:link rel="alternate" hreflang="x-default" href="'.$conf['base_url'].'/en/'.$shortname.'"/>'."\n";
    $sXML .= "  </url>\n";
    // /en/events/test
    $sXML .= "  <url>\n";
    $sXML .= "    <loc>".$conf['base_url']."/".$locale."/events/".$shortname."</loc>\n"; 
    foreach ($locales as $locale2) {
      $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'/events/'.$shortname.'"/>'."\n";
    }
    $sXML .= '      <xhtml:link rel="alternate" hreflang="x-default" href="'.$conf['base_url'].'/en/events/'.$shortname.'"/>'."\n";
    $sXML .= "  </url>\n";
    // /en/test/report
    $sXML .= "  <url>\n";
    $sXML .= "    <loc>".$conf['base_url']."/".$locale."/".$shortname."/report</loc>\n"; 
    foreach ($locales as $locale2) {
      $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'/'.$shortname.'/report"/>'."\n";
    }
    $sXML .= '      <xhtml:link rel="alternate" hreflang="x-default" href="'.$conf['base_url'].'/en/'.$shortname.'/report"/>'."\n";
    $sXML .= "  </url>\n";
  }
}

// For each event show max of 100 detailed records.
// TODO: We currently only show locally reported records.
foreach ($locales as $locale) {
  foreach ($shortnames as $shortname) {
    $q = "
      SELECT p.p_uuid
      FROM person_uuid p, incident i
      WHERE i.shortname = '".$shortname."'
      AND i.closed = 0 AND i.unlisted = 0
      AND p_uuid LIKE '%nlm.nih.gov%'
      AND p.incident_id = i.incident_id
      AND p.expiry_date > NOW() 
      LIMIT 100;
    ";
    $res = $global['db']->Execute($q);
    if($res === false) {
      die($global['db']->ErrorMsg());
    }
    $persons = [];
    while ($row = $res->FetchRow()) {
      $persons[] = $row['p_uuid'];
    }
    foreach ($persons as $person) {
      $recnum = str_replace(".", "#", substr($person, strpos($person, "record.")));
      $sXML .= "  <url>\n";
      $sXML .= "    <loc>".$conf['base_url']."/".$locale."/".$shortname."/".$recnum."/view</loc>\n"; 
      foreach ($locales as $locale2) {
        $sXML .= '      <xhtml:link rel="alternate" hreflang="'.$locale2.'" href="'.$conf['base_url'].'/'.$locale2.'/'.$shortname.'/'.$recnum.'/view"/>'."\n";
      }
      $sXML .= "  </url>\n";
    }
  }
}

// close
$sXML .= "</urlset>";

// out
ob_start("ob_gzhandler"); // enable gzip compression
header("Content-Type: application/xml; charset=utf-8");
header("Cache-Control: max-age=5"); // cache for 5 seconds
header("Expires: ".date('D, j M Y H:i:s')." GMT", time()+(5)); // expire in 5 seconds
echo $sXML;
ob_end_flush();
