<?
/**
 * @name     image lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// This function resizes a given image and saves it to a destination.
function taupo_image_resize($src_path, $desc_path, $width, $height, $jpegger = false) {
  $info = getimagesize($src_path);
  if(! $info)
  return false;
  $width_orig = $info[0];
  $height_orig = $info[1];
  if ($width && ($width_orig < $height_orig)) {
    $width = ($height / $height_orig) * $width_orig;
  } else {
    $height = ($width / $width_orig) * $height_orig;
  }
  $imageNew = imagecreatetruecolor($width, $height);
  $func = NULL;
  $func2 = "imagejpeg";
  list(,$mime) = explode("/",$info['mime']);
  $mime = strtolower($mime);
  if(stripos($mime,"bmp")!==FALSE){
    // bitmap
    $func =  "imagecreatefromwbmp";
    $func2 = "imagewbmp";
    $quality = null;
  } else if(stripos($mime,"png")!==FALSE){
    // png
    $func = "imagecreatefrompng";
    $func2 = "imagepng";
    $quality = 9;
  }else if(stripos($mime,"jpeg")!==FALSE){
    // jpeg and pjpeg
    $func = "imagecreatefromjpeg";
    $func2 = "imagejpeg";
    $quality = 100;
  }else if(stripos($mime,"gif")!==FALSE){
    // gif
    $func = "imagecreatefromgif";
    $func2 = "imagegif";
    $quality = null;
  }
  $image = NULL;
  if($func===NULL){
    $func = "imagecreate";
    $image = $func($width,$height);
  }else{
    $image = $func($src_path);
  }
  imagecopyresampled($imageNew, $image, 0, 0, 0, 0, $width, $height, $width_orig, $height_orig);
  if($jpegger == true) {
    $func2 = "imagejpeg";
    $quality = 75;
  }
  $func2($imageNew, $desc_path, $quality);
}

// This function can resize a given image (with only a height constraint) and save it to the given location.
 function taupo_image_resize_height($src_path, $desc_path, $height) {
  $info = getimagesize($src_path);
  if(! $info) {
    return false;
  }
  $width_orig = $info[0];
  $height_orig = $info[1];
  $width = ($height / $height_orig) * $width_orig;
  $imageNew = imagecreatetruecolor($width, $height);
  $func = NULL;
  $func2 = "imagejpeg";
  list(,$mime) = explode("/",$info['mime']);
  $mime = strtolower($mime);
  if(stripos($mime,"bmp")!==FALSE){
    // bitmap
    $func =  "imagecreatefromwbmp";
    $func2 = "imagewbmp";
    $quality = null;
  }else if(stripos($mime,"png")!==FALSE){
    // png
    $func = "imagecreatefrompng";
    $func2 = "imagepng";
    $quality = 9;
  }else if(stripos($mime,"jpeg")!==FALSE){
    // jpeg and pjpeg
    $func = "imagecreatefromjpeg";
    $func2 = "imagejpeg";
    $quality = 100;
  }else if(stripos($mime,"gif")!==FALSE){
    // gif
    $func = "imagecreatefromgif";
    $func2 = "imagegif";
    $quality = null;
  }
  $image = NULL;
  if($func===NULL){
    $func = "imagecreate";
    $image = $func($width,$height);
  }else{
    $image = $func($src_path);
  }
  imagecopyresampled($imageNew, $image, 0, 0, 0, 0, $width, $height, $width_orig, $height_orig);
  if ($quality != null) {
    $func2($imageNew, $desc_path, $quality);
  } else {
    $func2($imageNew, $desc_path);
  }
}

// This function can resize a given image (with only a width constraint) and save it to the given location.
 function taupo_image_resize_width($src_path, $desc_path, $width) {
  $info = getimagesize($src_path);
  if(! $info) {
    return false;
  }
  $width_orig = $info[0];
  $height_orig = $info[1];
  $height = ($width / $width_orig) * $height_orig;
  $imageNew = imagecreatetruecolor($width, $height);
  $func = NULL;
  $func2 = "imagejpeg";
  list(,$mime) = explode("/",$info['mime']);
  $mime = strtolower($mime);
  if(stripos($mime,"bmp")!==FALSE){
    // bitmap
    $func =  "imagecreatefromwbmp";
    $func2 = "imagewbmp";
    $quality = null;
  }else if(stripos($mime,"png")!==FALSE){
    // png
    $func = "imagecreatefrompng";
    $func2 = "imagepng";
    $quality = 9;
  }else if(stripos($mime,"jpeg")!==FALSE){
    // jpeg and pjpeg
    $func = "imagecreatefromjpeg";
    $func2 = "imagejpeg";
    $quality = 100;
  }else if(stripos($mime,"gif")!==FALSE){
    // gif
    $func = "imagecreatefromgif";
    $func2 = "imagegif";
    $quality = null;
  }
  $image = NULL;
  if($func===NULL){
    $func = "imagecreate";
    $image = $func($width,$height);
  }else{
    $image = $func($src_path);
  }
  imagecopyresampled($imageNew, $image, 0, 0, 0, 0, $width, $height, $width_orig, $height_orig);
  if ($quality != null) {
    $func2($imageNew, $desc_path, $quality);
  } else {
    $func2($imageNew, $desc_path);
  }
}
