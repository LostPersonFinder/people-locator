<?
/**
 * @name         Person Finder Interchange Format
 * @author       pl@miernicki.com
 * @about        Developed by the U.S. National Library of Medicine
 * @link         https://gitlab.com/tehk/people-locator
 * @license	     https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */


// WebImage -- a class to encapsulate the image table

require_once('lib_pfif.inc');

class WebImage {

   private $id = null;
   public $image = null;    // The binary image
   public $type = "";       // MIME type for image
   public $height = 0;      // The number of lines in the image
   public $width = 0;       // The number of pixels per line
   public $url = null;      // The source URL for the image
   private $initialized = false;

   public function __construct($url = null, $uuid = null) {
      // pfif_error_log('WebImage:'.$url.','.$uuid);
      if (isset($url) && isset($uuid)) {
         // pfif_error_log('WebImage:args ok');
         $this->set_id($uuid);
         $this->_init($url);
      } else {
         pfif_error_log('taupo_mod_pfif_WebImage: bad arguments (url = ' . $url . ' uuid= ' . $uuid . ')');
      }
   }

   public function is_initialized() {
      return $this->initialized;
   }

   public function set_id($uuid) {
      $this->id = $uuid;
   }

   public function set_image($url) {
      $status = false;
      if (isset($url)) {
         $this->_init($url);
         $status = true;
      }
      return $status;
   }

   private function _init($url) {
      // Check the URL to be sure it exists and is for a supported image type
      // Fetch the image
      $image_data = $this->curl_file_get_contents($url);
      if ($image_data === false) {
         //pfif_error_log('Fetching image failed for: ' . $url);
         return;
      }
		// Squelch warnings about empty or non-image file.
      $image_info = @getimagesizefromstring($image_data);
      //pfif_error_log('_init.image_info:'.var_export($image_info,true));

      $type = ($image_info != false) ? $image_info['mime'] : false;
      // pfif_error_log('_init.type:'.var_export($type,true));

      $is_valid_type = ($type != false) ? pfif_check_valid_image_type($type) : false;
      // pfif_error_log('is_valid_type:'.var_export($is_valid_type,true));

      // Set the image type and size.
      if ($is_valid_type) {
         $this->url = $url; //$saveFile;
         $this->image = $image_data; // null;
         $this->width = $image_info[0];
         $this->height = $image_info[1];
         list(, $this->type) = explode('/', $type);
         $this->channels = (isset($image_info['channels'])) ? (int)$image_info['channels'] : 3;
         $this->initialized = true;
         unset($image_data);
      } else {
         // pfif_error_log("Image get failed for ".$url);
      }
   }

   // Due to problems with file_get_contents(), use curl (PL-696).
   private function curl_file_get_contents($url) {
        $c = curl_init();
        curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($c, CURLOPT_URL, $url);
        $contents = curl_exec($c);
        curl_close($c);

        if ($contents) {
           return $contents;
        } else {
           return FALSE;
        }
    }
}
