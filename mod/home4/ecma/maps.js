'use strict';
// maps lib

// setup a reporting map in wizard
function initReportMap() {
  clog('%cmaps::initReportMap', 9, 'color: grey');
  if ($('.rmap').length) {
    // Reset map to initial props.
    window.rmap.language = window.router.locale;
    window.rmap.zoom = 15;
    window.rmap.longitude = window.longitude;
    window.rmapm.longitude = window.longitude;
    window.rmap.latitude = window.latitude;
    window.rmapm.latitude = window.latitude;
  } else {
    // Create initial map.
    var a = '<google-map class="rmap" language="speak" disable-default-ui api-key="zzzzz" latitude="lat00" longitude="lon00" zoom="15" fit-to-markers>'+
            '<google-map-marker class="rmapm" latitude="lat00" longitude="lon00" draggable="true"></google-map-marker>'+
          '</google-map>';
    var b = str_ireplace('zzzzz', window.gmapsapikey, a);
    var c = str_ireplace('lat00', window.latitude,  b);
    var d = str_ireplace('lon00', window.longitude, c);
    var e = str_ireplace('speak', window.router.locale, d);
    $('.rmaph').html(e);
    window.rmap  = document.querySelector('.rmap');
    window.rmapm = document.querySelector('.rmapm');
  }
}

// set map defaults
function defaultReportMap() {
  window.rmap.resize();
  clog('%cmaps::defaultReportMap', 9, 'color: grey');
}

// generate map for a record
function getRecordMap(lat, lon) {
  clog('%cmaps::getRecordMap lat:'+lat+' lon:'+lon, 9, 'color: grey');
  var a = '<google-map class="smap" id="lkl" language="speak" api-key="zzzzz" latitude="lat00" longitude="lon00" disable-default-ui '+
            'additional-map-options=\'{"scrollwheel":"false"}\' min-zoom="4" max-zoom="16" zoom="14">'+
            '<google-map-marker latitude="lat00" longitude="lon00" title="" draggable="false" drag-events></google-map-marker>'+
          '</google-map>';
  var b = str_ireplace('zzzzz', window.gmapsapikey, a);
  var c = str_ireplace('lat00', lat, b);
  var d = str_ireplace('lon00', lon, c);
  var e = str_ireplace('speak', window.router.locale, d);
  return e;
}

// record map destructor
function destructRecordMap() {
  $('.record-map-d').html('');
  clog('%cmaps::destructRecordMap', 9, 'color: grey');
}
