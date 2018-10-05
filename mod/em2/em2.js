/**
 * @name     Event Manager 2
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

//'use strict';

function em2_reset_lS() {
  localStorage.removeItem('eventData');
  localStorage.removeItem('eventDataTime');
  console.log('%cLOCAL EVENT DATA CACHE CLEARED!', 'color: red');
}

em2_show_events();

function em2_ask_delete(incident_id) {
  window.delete_incident_id = incident_id;
  $("#deleteOk").removeClass('none');
  document.querySelector('#deleteOk').open();
}
function em2_delete_event_ok() {
  em2_delete(window.delete_incident_id);
}

function em2_ask_purge(incident_id) {
  window.purge_incident_id = incident_id;
  $("#purgeOk").removeClass('none');
  document.querySelector('#purgeOk').open();
}
function em2_purge_event_ok() {
  em2_purge(window.purge_incident_id);
}

function em2_delete(incident_id) {
  $('#toast2').prop('duration', 10000000);
  toast2('Deleting Event #'+incident_id+' ... (this may take a long time on large events)');
  em2_perform_delete(incident_id);
}

function em2_purge(incident_id) {
  $('#toast2').prop('duration', 10000000);
  toast2('Purging Event #' + incident_id + ' ... (this may take a long time on large events)');
  em2_perform_purge(incident_id);
}

function em2_sort_events(col, showArchived) {
  // Decode 'down' and 'up' to compare against current direction.
  var down = $("<div/>").html("&#x25BC;").text();
  var up = $("<div/>").html("&#x25B2;").text();
   var direction = '';
  // Set desired direction.
  if ($("#eventsCol"+col).html() == down) {
    // Reverse direction.
    direction = "up";
  } else if ($("#eventsCol"+col).html() == up) {
    // Reverse direction.
    direction = "down";
  } else if (col == 'date') {
    direction = "up";
  } else {
    direction = "down";
  }
  // Display events properly sorted.
  em2_show_events(col, direction, showArchived);
}

// Filter event listing based on whether we are showing archived events or not.
function em2_filter_events(col, direction) {
  em2_show_events(col, direction, (($('#showArchived').prop('checked'))? 1 : 0));
}

function go_reunite_msg() {
  var msg = '';
  $('#reunitemsgbutton').css('opacity', 0.2);
  $("#reunitemsgbutton").prop('disabled', true);
  msg = $('#reunitemsg').val();
  em2_reunite_msg(msg);
}

// Google Maps junx
google.maps.visualRefresh = true;
var latitude = '';
var longitude = '';
var geocoder;
var map;
var marker;
function getLocation(pos) {
  latitude = pos.coords.latitude;
  longitude = pos.coords.longitude;
  load_map();
  geocoder.geocode({'latLng': marker.getPosition()}, function(results, status) {
    if (status == google.maps.GeocoderStatus.OK) {
      if (results[0]) {
        $('#address').val(results[0].formatted_address);
        $('#latitude').val(marker.getPosition().lat());
        $('#longitude').val(marker.getPosition().lng());
      }
    }
  });
}

function unknownLocation() {
  alert('Could not find location');
}

function detect_load() {
  navigator.geolocation.getCurrentPosition(getLocation, unknownLocation);
}

function load_map(latitude, longitude, street) {
  if(latitude == null || longitude == null) {
    latitude = 39;
    longitude = -77.101;
  }
  var latlng = new google.maps.LatLng(latitude, longitude);
  var config = {
    zoom: 8,
    center: latlng,
    navigationControl: true,
    navigationControlOptions: {
      style: google.maps.NavigationControlStyle.ZOOM_PAN
    },
    mapTypeControl: true,
    mapTypeControlOptions: {
      style: google.maps.MapTypeControlStyle.DROPDOWN_MENU
    },
    mapTypeId: google.maps.MapTypeId.ROADMAP,
    scrollwheel: false
  };
  map = new google.maps.Map(document.getElementById("mapCanvas"), config);
  geocoder = new google.maps.Geocoder();
  marker = new google.maps.Marker({
    map: map,
    draggable: true
  });
  $(function() {
    $("#address").autocomplete({
      // This bit uses the geocoder to fetch address values
      source: function(request, response) {
        geocoder.geocode( {'address': request.term }, function(results, status) {
          response($.map(results, function(item) {
            return {
              label:  item.formatted_address,
              value: item.formatted_address,
              latitude: item.geometry.location.lat(),
              longitude: item.geometry.location.lng()
            }
          }));
        })
      },
      // This bit is executed upon selection of an address
      select: function(event, ui) {
        $("#latitude").val(ui.item.latitude);
        $("#longitude").val(ui.item.longitude);
        var location = new google.maps.LatLng(ui.item.latitude, ui.item.longitude);
        marker.setPosition(location);
        map.setCenter(location);
      }
    });
  });
  // Add listener to marker for reverse geocoding
  google.maps.event.addListener(marker, 'drag', function() {
    geocoder.geocode({'latLng': marker.getPosition()}, function(results, status) {
      if (status == google.maps.GeocoderStatus.OK) {
        if (results[0]) {
          $('#address').val(results[0].formatted_address);
          $('#latitude').val(marker.getPosition().lat());
          $('#longitude').val(marker.getPosition().lng());
        }
      }
    });
  });
  $("#latitude").val(latitude);
  $("#longitude").val(longitude);
  $("#address").val(street);
  var location = new google.maps.LatLng(latitude, longitude);
  marker.setPosition(location);
  map.setCenter(location);
}

function em2_get_data() {
  var r = new Object();
  r.latitude         = $("#latitude"             ).val();
  r.longitude        = $("#longitude"            ).val();
  r.longName         = $("#longName"             ).val();
  r.shortName        = $("#shortName"            ).val();
  r.eventParent      = $("#eventParent"          ).val();
  r.eventType        = $("#eventType"            ).val();
  r.eventVisibility  = $("#eventVisibility"      ).val();
  r.eventDate        = $("#eventDate"            ).val();
  r.eventId          = $("#eventId"              ).val();
  r.pfKey            = $("#pfKey"                ).val();
  r.pfSync           = $("#pfSync:checked"       ).val();
  r.eventArchived    = $("#eventArchived:checked").val();
  r.eventUnlisted    = $("#eventUnlisted:checked").val();
  r.street           = $("#address"              ).val();
  r.eventClosed      = $("#eventClosed:checked"  ).val();
  r.article          = $("#article"              ).val();
  r.images           = $("#images"               ).val();
  r.caption          = $("#caption"              ).val();
  r.tags             = $("#tags"                 ).val();
  r.adminotes        = $("#adminotes"            ).val();
  r.lang             = $("#lang_prev"            ).val();
  if($("#eventDefault:checked").val() == "default") {
    r.eventDefault = 1;
  } else {
    r.eventDefault = 0;
  }
  var rj = JSON.stringify(r);
  return(rj);
}

// from http://goo.gl/CLJxF
function htmlspecialchars(string, quote_style, charset, double_encode) {
  var optTemp = 0, i = 0,
  noquotes = false;
  if (typeof quote_style === 'undefined' || quote_style === null) {
    quote_style = 2;
  }
  string = string.toString();
  if (double_encode !== false) {
    // Put this first to avoid double-encoding
    string = string.replace(/&/g, '&amp;');
  }
  string = string.replace(/</g, '&lt;').replace(/>/g, '&gt;');
  var OPTS = {
    'ENT_NOQUOTES': 0,
    'ENT_HTML_QUOTE_SINGLE': 1,
    'ENT_HTML_QUOTE_DOUBLE': 2,
    'ENT_COMPAT': 2,
    'ENT_QUOTES': 3,
    'ENT_IGNORE': 4
  };
  if (quote_style === 0) {
    noquotes = true;
  }
  if (typeof quote_style !== 'number') {
    // Allow for a single string or an array of string flags
    quote_style = [].concat(quote_style);
    for (i = 0; i < quote_style.length; i++) {
      // Resolve string input to bitwise e.g. 'PATHINFO_EXTENSION' becomes 4
      if (OPTS[quote_style[i]] === 0) {
        noquotes = true;
      } else if (OPTS[quote_style[i]]) {
        optTemp = optTemp | OPTS[quote_style[i]];
      }
    }
    quote_style = optTemp;
  }
  if (quote_style & OPTS.ENT_HTML_QUOTE_SINGLE) {
    string = string.replace(/'/g, '&#039;');
  }
  if (!noquotes) {
    string = string.replace(/"/g, '&quot;');
  }
  return string;
}




function em2_get_image(i) {
  $('#uploadProgress').html('UPLOADING...');
  if (window.File && window.FileReader && window.FileList && window.Blob) {
  } else {
    alert('Please update to a modern browser like Firefox or Chrome that supports FileReader/HTML5 !');
    return;
  }   
  input = document.getElementById('add_image');
  if (!input) {
    alert("Um, couldn't find the fileinput element.");
    $('#uploadProgress').html('');
  } else if (!input.files) {
    alert('Please update to a modern browser like Firefox or Chrome that supports FileReader/HTML5 !');
    $('#uploadProgress').html('');
  } else if (!input.files[0]) {
    alert("Please make sure to CHOOSE AN IMAGE");               
    $('#uploadProgress').html('');
  } else {
    window.incident_id = i;
    file = input.files[0];
    fr = new FileReader();
    fr.onload = receivedText;
    fr.readAsBinaryString(file);
    var di = document.getElementById("DELIMAGE");
    di.disabled = false;
    di.style.opacity = '1';
  }
}
function receivedText() {
  em2_image_send(window.incident_id, base64_encode(fr.result));
}
function em2_del_image() {
  $('#uploadProgress').html('');
  $('#images').val('');
  var di = document.getElementById("DELIMAGE");
  di.disabled = true;
  di.style.opacity = '0.2';
}


function base64_encode (data) {
  var b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  var o1, o2, o3, h1, h2, h3, h4, bits, i = 0,
    ac = 0,
    enc = "",
    tmp_arr = [];
  if (!data) {
    return data;
  }
  do { // pack three octets into four hexets
    o1 = data.charCodeAt(i++);
    o2 = data.charCodeAt(i++);
    o3 = data.charCodeAt(i++);
    bits = o1 << 16 | o2 << 8 | o3;
    h1 = bits >> 18 & 0x3f;
    h2 = bits >> 12 & 0x3f;
    h3 = bits >> 6 & 0x3f;
    h4 = bits & 0x3f;
    // use hexets to index into b64, and append result to encoded string
    tmp_arr[ac++] = b64.charAt(h1) + b64.charAt(h2) + b64.charAt(h3) + b64.charAt(h4);
  } while (i < data.length);
  enc = tmp_arr.join('');
  var r = data.length % 3;
  return (r ? enc.slice(0, r - 3) : enc) + '==='.slice(r || 3);
}


function updateImage(url) {
  if(url == "INVALID_IMAGE") {
    $('#images').val('');
    $('#uploadProgress').html('INVALID IMAGE TYPE. Please choose a jpeg, gif, or png.');
    var di = document.getElementById("DELIMAGE");
    di.disabled = true;
    di.style.opacity = '0.1';
  } else {
    $('#images').val(url);
    $('#uploadProgress').html('<img src="'+url+'">');
    var di = document.getElementById("DELIMAGE");
    di.disabled = false;
    di.style.opacity = '1';
  }
}


// PHPJS functions
function strlen(string) {
  return string.length;
}


function explode (delimiter, string, limit) {
  if ( arguments.length < 2 || typeof delimiter === 'undefined' || typeof string === 'undefined' ) return null;
  if ( delimiter === '' || delimiter === false || delimiter === null) return false;
  if ( typeof delimiter === 'function' || typeof delimiter === 'object' || typeof string === 'function' || typeof string === 'object'){
    return { 0: '' };
  }
  if ( delimiter === true ) delimiter = '1';
  // Here we go...
  delimiter += '';
  string += '';
  var s = string.split( delimiter );
  if ( typeof limit === 'undefined' ) return s;
  // Support for limit
  if ( limit === 0 ) limit = 1;
  // Positive limit
  if ( limit > 0 ){
    if ( limit >= s.length ) return s;
    return s.slice( 0, limit - 1 ).concat( [ s.slice( limit - 1 ).join( delimiter ) ] );
  }
  // Negative limit
  if ( -limit >= s.length ) return [];
  s.splice( s.length + limit );
  return s;
}



// jQuery TextChange Plugin
(function ($) {
  $.event.special.textchange = {
    setup: function (data, namespaces) {
    $(this).data('lastValue', this.contentEditable === 'true' ? $(this).html() : $(this).val());
      $(this).bind('keyup.textchange', $.event.special.textchange.handler);
      $(this).bind('cut.textchange paste.textchange input.textchange', $.event.special.textchange.delayedHandler);
    },
    teardown: function (namespaces) {
      $(this).unbind('.textchange');
    },
    handler: function (event) {
      $.event.special.textchange.triggerIfChanged($(this));
    },
    delayedHandler: function (event) {
      var element = $(this);
      setTimeout(function () {
        $.event.special.textchange.triggerIfChanged(element);
      }, 25);
    },
    triggerIfChanged: function (element) {
      var current = element[0].contentEditable === 'true' ? element.html() : element.val();
      if (current !== element.data('lastValue')) {
        element.trigger('textchange',  [element.data('lastValue')]);
        element.data('lastValue', current);
      }
    }
  };
  $.event.special.hastext = {
    setup: function (data, namespaces) {
      $(this).bind('textchange', $.event.special.hastext.handler);
    },
    teardown: function (namespaces) {
      $(this).unbind('textchange', $.event.special.hastext.handler);
    },
    handler: function (event, lastValue) {
      if ((lastValue === '') && lastValue !== $(this).val()) {
        $(this).trigger('hastext');
      }
    }
  };
  $.event.special.notext = {
    setup: function (data, namespaces) {
      $(this).bind('textchange', $.event.special.notext.handler);
    },
    teardown: function (namespaces) {
      $(this).unbind('textchange', $.event.special.notext.handler);
    },
    handler: function (event, lastValue) {
      if ($(this).val() === '' && $(this).val() !== lastValue) {
        $(this).trigger('notext');
      }
    }
  };  
})(jQuery);


function saveCheck(newVar) {
  toast2('Canceling edit, no changes saved...');
  em2_show_events();
}

function em2_change_lang(incident_id, is_new) {
  // Reload with new language.
  em2_perform_edit(incident_id, $('#lang').val());
}
