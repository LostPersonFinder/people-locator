'use strict';

// photo util library
window.cameraError = false;
window.image = false;

// grabs the camera feed from the browser
function getCamera() {
  if(!window.cameraError) {
    navigator.getUserMedia  = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
    var video = document.querySelector('video');
    if(navigator.getUserMedia) {
      // first try high def
      navigator.getUserMedia(
        {audio: false, video: {mandatory: {minWidth: 1280, minHeight: 720}}},
        function(stream) {
          video.src = window.URL.createObjectURL(stream);
          showCapture();
        },
        // fail high def try low def
        function(stream) {
          navigator.getUserMedia(
            {audio: false, video: {mandatory: {minWidth: 640, minHeight: 480}}},
            function(stream) {
              video.src = window.URL.createObjectURL(stream);
              showCapture();
            },
            camError // fail low def too
          );
        }
      );
    } else {
      camError();
    }
  }
}
// show captured video stream
function showCapture() {
  $('.video'        ).removeClass('none');
  $('.buttonCapture').removeClass('none');
  $('.buttonDelete' ).removeClass('none');
  $('.buttonCamera').addClass('none');
  $('.buttonSelect').addClass('none');
}
// handle camera error
function camError() {
  $('.buttonCamera').addClass('zero');
  toastYellow('cameraError', window.toastDuration);
  window.cameraError = true;
}
// capture image from camera
function capturePhoto() {
  var video  = document.querySelector('video');
  var canvas = document.querySelector('canvas');
  //
  var ctx = canvas.getContext('2d');
  canvas.width  = video.videoWidth;
  canvas.height = video.videoHeight;
  clog('image resolution >> '+canvas.width+'x'+canvas.height, 9);
  ctx.drawImage(video, 0, 0);
  loadImage(canvas.toDataURL('image/jpeg',0.75));
}
// load image to browser
function loadImage(imgData) {
  // find zone resolution
  var zoneE  = document.querySelector('.dropzone');
  window.zoneW = parseInt(window.getComputedStyle(zoneE).width);
  window.zoneH = parseInt(window.getComputedStyle(zoneE).height);
  clog('zone resolution >> '+window.zoneW+'x'+window.zoneH, 9);
  // load temporary in-memory image to get accurate width/height and then display it
  $("<img/>")
  .attr("src", imgData)
  .on("load", function() {
    var iw = this.width;
    var ih = this.height;
    var wp = 0; // width padding
    var hp = 0; // height padding
    var cI = '';  // property to change (height or width)
    var cw = window.zoneW;
    var ch = window.zoneH;
    // if image aspect ratio wider than our canvas' (wider)
    if((iw / ih) >= (cw / ch)) {
      hp = Math.floor((ch - (cw / iw * ih)) / 2);
      cI = 'width';
      // else if image aspect ratio narrower than our canvas' (taller)
    } else if((iw / ih) < (cw / ch)) {
      wp = Math.floor((cw - (ch / ih * iw)) / 2);
      cI = 'height';
    }
    var newPad = String(hp)+'px '+String(wp)+'px';
    // gen random img id
    var newId = 'tmpImageId'+String(Math.floor(Math.random()*1000000000));
    // clear html node and insert new image
    $('.showImage').html('<img id="'+newId+'" src="'+imgData+'">');
    $('.showImage').removeClass('none');
    // try resize image and padding
    try {
      $('#'+newId).css(cI,'100%');
      $('#'+newId).css("margin",newPad);
    // fallback to aspect-ratio incorrect display on error
    } catch(e) {
      clog('error on padding; retrying...', 9);
      $('#'+newId).css('width', cw+'px');
      $('#'+newId).css('height',ch+'px');
    }
    // update buttons
    $('.video').addClass('none');
    $('.buttonCapture').prop('disabled', true);
    $('.buttonCapture').addClass('fixButtons');
    $('.buttonDelete').addClass('fixButtons');
    $('.buttonSelect').addClass('fixButtons');
    $('.buttonRotateL').removeClass('none');
    $('.buttonRotateR').removeClass('none');
    $('.buttonMoveU').removeClass('none');
    $('.buttonMoveD').removeClass('none');
    $('.buttonDelete' ).prop('disabled', false);
    $('.buttonRotateL').prop('disabled', false);
    $('.buttonRotateR').prop('disabled', false);
    $('.buttonMoveU').prop('disabled', false);
    $('.buttonMoveD').prop('disabled', false);
    $('.photoCapText' ).removeClass('zero');
    // start cropper
    startCropper();
    window.image = true;
  });
}
// allow cropping
function startCropper() {
  // init cropper
  window.imageCrop = $('.showImage > img').cropper({
    strict: true,
    responsive: true,
    modal: true,
    guides: true,
    movable: true,
    scalable: false,
    zoomable: false,
    mouseWheelZoom: false,
    touchDragZoom: true,
    doubleClickToggle: false,
    minCropBoxWidth: 160,
    minCropBoxHeight: 90,
    autoCropArea: .95,
    crop: function(e) {
      /*
      clog(e.x, 9);
      clog(e.y, 9);
      clog("width: "+e.width, 9);
      clog("height: "+e.height, 9);
      clog(e.rotate, 9);
      clog(e.scaleX, 9);
      clog(e.scaleY, 9);
      */
    }
  });
}
// rotate left 10 degrees
function rotateL() {
  window.imageCrop.cropper('rotate', -10);
}
// rotate right 10 degrees
function rotateR() {
  window.imageCrop.cropper('rotate', 10);
}
// move up 10 pixels 
function moveU() {
  window.imageCrop.cropper('move', 0, -10);
}
// move down 10 pixels 
function moveD() {
  window.imageCrop.cropper('move', 0, 10);
}
function getCroppedImage() {
  if(window.image === true) {
    // get cropped photo data
    window.r.photo = window.imageCrop.cropper('getCroppedCanvas').toDataURL('image/jpeg',0.75);
    //window.r.photo = window.imageCrop.cropper('getCroppedCanvas').toDataURL('image/webp',0.75);
    // remove all but binary64 image data
    window.r.photo = window.r.photo.replace('data:image/jpeg;base64,', '');
    //window.r.photo = window.r.photo.replace(/^data:image\/(png|jpeg|gif);base64,/, '');
  }
}
// fired when a user select an image from device
function selectFile(thefiles) {
  var newId;
  var file = thefiles[0];
  // only supported formats
  if(file.type.match('image/tiff') || file.type.match('image/jpeg') || file.type.match('image/png')) {
    var picReader = new FileReader();
    picReader.addEventListener("load", function(event) {
      var picFile = event.target;
      loadImage(picFile.result);
    });
    // read the image
    picReader.readAsDataURL(file);
    // update ui
    $('.buttonDelete' ).removeClass('none');
    $('.buttonRotateL').removeClass('none');
    $('.buttonRotateR').removeClass('none');
    $('.buttonMoveU').removeClass('none');
    $('.buttonMoveD').removeClass('none');
    $('.buttonCamera').addClass('none');
    $('.buttonSelect').prop('disabled', true);
    // PL-1671
    $('#filesid').addClass('none');
  } else {
    toastYellow('photoOnly', window.toastDuration);
    deletePhoto();
  }
}
// delete photo and restore buttons
function deletePhoto() {
  // update ui
  $('.buttonDelete' ).prop('disabled', true);
  $('.buttonDelete' ).addClass('none');
  $('.buttonRotateL').addClass('none');
  $('.buttonRotateR').addClass('none');
  $('.buttonMoveU').addClass('none');
  $('.buttonMoveD').addClass('none');
  $('.buttonCapture').addClass('none');
  $('.buttonCamera').removeClass('none');
  $('.buttonSelect').removeClass('none');
  $('.buttonSelect').prop('disabled', false);
  $('.buttonCapture').removeClass('fixButtons');
  $('.buttonDelete').removeClass('fixButtons');
  $('.buttonSelect').removeClass('fixButtons');
  $('.photoCapText' ).addClass('zero');
  $('.buttonCapture').prop('disabled', false);
  $('.showImage').html('');
  $('.showImage').addClass('none');
  // reset
  $('.files').val('');
  if(typeof window.imageCrop !== 'undefined') {
    window.imageCrop.cropper('destroy');
  }
  window.image = false;
  // PL-1671
  if(window.iOSSafari) {
    clog('%creport::goReport iOS_SAFARI 🐳', 9, 'color: pink');
    $('#pbselect').prop('disabled', true);
    $('#pbselect').addClass('none');
    $('#filesid').removeClass('none');
    $('.buttonCamera').prop('disabled', true);
    $('.buttonCamera').addClass('none');
  }
}
