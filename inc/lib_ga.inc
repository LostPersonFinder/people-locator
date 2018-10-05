<?
/**
 * @name     Google Analytics Direct Reporting Library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// 
function detectVisit() {
  $agent = strtolower($_SERVER['HTTP_USER_AGENT']);
  $bot_strings = array(
  "google", "bot", "yahoo", "spider", "archiver", "curl",
     "python", "nambu", "twitt", "perl", "sphere", "pear",
     "java", "wordpress", "radian", "crawl", "yandex", "eventbox",
     "monitor", "mechanize", "facebookexternal", "bingbot",
     "googlebot-image", "googlebot"
  );
  foreach($bot_strings as $bot) {
    if(strpos($agent, $bot) !== false) {
      return "bot";
    }
  }
  return "normal";
}

// make a tracking request to Google Analytics from this server
function sendRequestToGoogleAnalytics($url, $params) {
  $postData = http_build_query($params);
  $options = 
    array(
      "http" => array(
        "method" => "POST",
        "header"  => "Content-type: application/x-www-form-urlencoded",
        "content" => $postData
    )
  );
  file_get_contents($url, false, stream_context_create($options));
}

// have Google Analytics track this pageview // uses the measurement protocol https://developers.google.com/analytics/devguides/collection/protocol/v1
function taupo_ga_trackPageView() {
  global $conf;
  // since we aren't weeding out bots by virtue of using JavaScript.
  if (detectVisit() !== "normal") { return; }
  // get referrer
  $documentReferer = $_SERVER["HTTP_REFERER"];
  if (empty($documentReferer) && $documentReferer !== "0") {
    $documentReferer = "-";
  } else {
    $documentReferer = urldecode($documentReferer);
  }
  $documentPath = $_SERVER["REQUEST_URI"];
  if (empty($documentPath)) {
    $documentPath = "";
  } else {
    $documentPath = urldecode($documentPath);
  }
  $userAgent = $_SERVER["HTTP_USER_AGENT"];
  if (empty($userAgent)) {
    $userAgent = "";
  }
  $cid = 555; // Anonymous client ID.
  $url = "http://www.google-analytics.com/collect";
  preg_match('/(UA-[0-9]+-1)/', GOOGLE_UA, $matches);
  $tid = $matches[1];
  $params = array(
    "v" => 1,
    "t" => "pageview",
    "cid" => $cid,
    "ua" => urlencode($_SERVER["HTTP_USER_AGENT"]),
    "dh" => $_SERVER["SERVER_NAME"],
    "dr" => urlencode($documentReferer),
    "dp" => urlencode($documentPath),
    "tid" => $tid,
    "uip" => get_client_ip()
  );
  sendRequestToGoogleAnalytics($url, $params);
}
