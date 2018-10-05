<?
/**
 * @name     Taupo Constants
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// module long names
$conf['mod_abuse_name']  = "Abuse";
$conf['mod_admin_name']  = "Administration";
$conf['mod_arrive_name'] = "Arrivals";
$conf['mod_biu_name']    = "Batch Image Upload";
$conf['mod_br_name']     = "Batch Reporter";
$conf['mod_em2_name']    = "Event Manager";
$conf['mod_email_name']  = "Emailer";
$conf['mod_feed_name']   = "PFIF FEED web service";
$conf['mod_fm_name']     = "FaceMatch Admin";
$conf['mod_home4_name']  = "HOME4";
$conf['mod_load_name']   = "LOADer";
$conf['mod_notify_name'] = "Notify";
$conf['mod_pfif_name']   = "PFIF (Import/Export)";
$conf['mod_push_name']   = "Push Notification Weekly Test";
$conf['mod_quick_name']  = "Quick Comment Search";
$conf['mod_rest_name']   = "REST web services";
$conf['mod_solr_name']   = "SOLR";
$conf['mod_tally_name']  = "Social Tally (api key required)";
$conf['mod_test_name']   = "Test module";
$conf['mod_unit_name']   = "Unit Tests (api key required)";
$conf['mod_widget_name'] = "Widget";
$conf['mod_xst_name']    = "XAJAX Streamer";

// module ACL // admin only modules // ADMIN:1
$conf['acl']['abuse']  = array(1);
$conf['acl']['admin']  = array(1);
$conf['acl']['arrive'] = array(1);
$conf['acl']['biu']    = array(1);
$conf['acl']['br']     = array(1);
$conf['acl']['em2']    = array(1);
$conf['acl']['fm']     = array(1);
$conf['acl']['notify'] = array(1);
$conf['acl']['pfif']   = array(1);
$conf['acl']['email']  = array(1);
$conf['acl']['quick']  = array(1);
$conf['acl']['solr']   = array(1);
$conf['acl']['test']   = array(1);
$conf['acl']['xst']    = array(1);

// module ACL // public modules // ADMIN:1 USER:2 ANONYMOUS:3
$conf['acl']['feed']   = array(1,2,3);
$conf['acl']['home4']  = array(1,2,3);
$conf['acl']['load']   = array(1,2,3);
$conf['acl']['push']   = array(1,2,3);
$conf['acl']['rest']   = array(1,2,3);
$conf['acl']['tally']  = array(1,2,3);
$conf['acl']['widget'] = array(1,2,3);
$conf['acl']['unit']   = array(1,2,3);

# token/session constants
define('MAX_ALIVE',     31536000); // cookie token session expiration of 1 year
define('MAX_ALIVE_ANON', 3283200); // cookie token session expiration of 38 days (anon)
define('MAX_TOKENS',          16); // max number tokens a user can hold at once

// fail2ban ban duration
define("BAN_SECONDS", 60*60);    // 1 hour
define("MAX_AUTH_FAILURES", 16); // maximum threshold for fail2ban

# pass policy
define('PASS_MIN_CHARS', 8);
define('PASS_MAX_CHARS', 16);
define('PASS_HAS_UPPER', true);
define('PASS_HAS_LOWER', true);
define('PASS_HAS_DIGIT', true);
define('PASS_HAS_SPECL', false);
define('PASS_HAS_USRNM', true);

// pass policy errors
define("LESS_MIN_CHARS",    2);
define("MORE_MAX_CHARS",    3);
define("REQ_UPPERCASE",     4);
define("REQ_NUMBERS",       5);
define("REQ_SPECIAL_CHARS", 6);
define("HAS_USER",          7);
define("REQ_LOWERCASE",     9);

// user classes
define("ADMIN",      1);
define("REGISTERED", 2);
define("ANONYMOUS",  3);

// FM constants for source
define("UNKNOWN_SOURCE",        -1);
define("LOCAL_SOURCE",           0);
define("WEBSITE_SOURCE",         1);
define("REUNITE_IOS_SOURCE",     2);
define("REUNITE_ANDROID_SOURCE", 3);

// age minimum and maximum
define("MINIMUM_AGE", 0);
define("MAXIMUM_AGE", 120);

// FM constants for call
define("INGEST_SERVICE", 0);
define("QUERY_SERVICE",  1);
define("REMOVE_SERVICE", 2);

// GPF endpoint
define('GOOGLE_PF', 'https://google.org/personfinder/');

// Facematch web service constants
if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
  define("NO_FACES_IN_IMAGE", 1703);
} else {
  define("SUCCESS", 0);
  define("FM_NOT_AVAILABLE", 1);
  define("INVALID_KEY", 2);
  define("KEY_REQUIRED", 3);
  define("ILLEGAL_EVENT_ID", 4);
  define("EVENT_ID_NOT_FOUND", 5);
  define("CANNOT_DOWNLOAD_URL", 6);
  define("URL_IS_EMPTY", 7);
  define("URL_IS_NULL", 8);
  define("URL_BAD_SCHEME", 9);
  define("URL_IS_INVALID", 10);
  define("LOCAL_FILE_URL_NOT_ALLOWED", 11);
  define("NO_FACE_DETECTED", 12);
  define("ILLEGAL_INFLATE_PERCENTAGE", 13);
  define("REGIONS_PARAM_IS_NULL", 14);
  define("DISPLAY_REGS_PARAM_IS_NULL", 15);
  define("RESULT_PARAM_IS_NULL", 16);
  define("BAD_PERFORMANCE_VALUE", 17);
  define("FM_EXCEPTION", 18);
  define("FM_INTERNAL_ERROR", 19);
  define("FM_UNKNOWN", 20);
}
