/**
 * @name     BatchImageUploader
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function createRequestObject() {
  var http;
  if (navigator.appName == "Microsoft Internet Explorer") {
    http = new ActiveXObject("Microsoft.XMLHTTP");
  }
  else {
    http = new XMLHttpRequest();
  }
  return http;
}

function sendRequest() {
  var http = createRequestObject();
  http.open("GET", "index.php?mod=biu&act=progress");
  http.onreadystatechange = function () { handleResponse(http); };
  http.send(null);
}

function handleResponse(http) {
  var response;
  if (http.readyState == 4) {
    response = http.responseText;
    document.getElementById("bar_color").style.width = response + "%";
    document.getElementById("status").innerHTML = response + "%";
    if (response < 100) {
      setTimeout("sendRequest()", 1000);
    }
    else {
      document.getElementById("status").innerHTML = "100% Complete.";
    }
  }
}

function startUpload() {
  setTimeout("sendRequest()", 1000);
}

function enableUploadButton() {
	var d = document.getElementById("startupload");
	d.disabled = false;
	d.style.opacity = "1";
}

(function () { document.getElementById("myForm").onsubmit = startUpload; })();
