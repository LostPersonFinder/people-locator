// taupo push lib
'use strict';
window.subRetry = 1000;

// called at app startup
function notifyStart() {
  clog('%cpush::notifyStart', 9, 'color: #ff4081');
  if('serviceWorker' in navigator) {} else { unSupported(); return; } // not suppported
}

// called once the service worker is initialized
function notify_initialiseState() {
  clog('%cpush::initialiseState', 9, 'color: #ff4081');
  // notifications not supported
  if(!('showNotification' in ServiceWorkerRegistration.prototype)) { unSupported(); return; }
  // blocked permission
  if(Notification.permission === 'denied') { disAllowed(false); return; }
  // push messaging not supported
  if(!('PushManager' in window)) { unSupported(); return; }
  // callback fired when token is updated
  window.messaging.onTokenRefresh(function() {
    window.messaging.getToken().then(function(refreshedToken) {
      clog('%cpush::initialiseState token refreshed!', 9, 'color: #ff4081');
      sendSubscriptionToServer(refreshedToken);
    })
    .catch(function(err) {
      toastRed('restError9999', window.toastDuration);
      clog('%cpush::initialiseState -> error: '+err, 9, 'color: #ff4081');
    });
  });
}

// subscribe to push notifications
function subscribe() {
  clog('%cpush::subscribe', 9, 'color: #ff4081');
  window.subRetry = window.subRetry * 2; // double retry timeout
  window.messaging.requestPermission().then(function() {
    clog('%cpush::subscribe notification permission granted', 9, 'color: #ff4081');
    window.messaging.getToken().then(function(currentToken) {
      if(currentToken) {
        clog('%cpush::subscribe current token: '+currentToken, 9, 'color: #ff4081');
        sendSubscriptionToServer(currentToken);
        ga('send', 'pageview', window.location.pathname + '/allowpush');
      } else {
        clog('%cpush::subscribe no token', 9, 'color: #ff4081');
        setTimeout(function() { subscribe(); }, window.subRetry); // this fails sometimes, so retry w/ backing off delay
      }
    }).catch(function(err) {
      clog('%cpush::subscribe error while retrieving token: '+err, 9, 'color: #ff4081');
      setTimeout(function() { subscribe(); }, window.subRetry); // this fails sometimes, so retry w/ backing off delay
    });
  }).catch(function(err) { disAllowed(true); }); // no permission
}

// send rid to server
function sendSubscriptionToServer(subscription) {
  clog('%cpush::sendSubscriptionToServer', 9, 'color: #ff4081');
  // call web service
  var ru   = new Object();
  ru.token = window.token;
  ru.call  = 'push';
  ru.rid   = subscription;
  ru.about = 'web_browser/'+window.uap.getBrowser().name+'/'+window.uap.getBrowser().version+'__'+window.uap.getOS().name+'/'+window.uap.getOS().version;
  ru.sub   = true;
  var ruj  = JSON.stringify(ru);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    clog('%cpush::sendSubscriptionToServer -> response:'+e.target.status, 9, 'color: #ff4081');
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) { clog('%cpush::sendSubscriptionToServer -> success!', 9, 'color: #ff4081'); }
    // failure
    else {
      clog('%cpush::sendSubscriptionToServer -> failure! error:'+resp.error, 9, 'color: #ff4081');
      toastRed('restError9999', window.toastDuration);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(ruj);
}

// deactivate toggler
function unSupported() {
  clog('%cpush::unSupported', 9, 'color: #ff4081');
  $('#recordP').prop('active', false);
  $('#recordP').prop('disabled', true);
  $('#recordP').css('opacity', '0.2');
  $('#eventP').prop('active', false);
  $('#eventP').prop('disabled', true);
  $('#eventP').css('opacity', '0.2');
  $('#adminP').prop('active', false);
  $('#adminP').prop('disabled', true);
  $('#adminP').css('opacity', '0.2');
}

// permission blocked
function disAllowed(show) {
  clog('%cpush::unAllowed', 9, 'color: #ff4081');
  if(show) { toastYellow('pushNotificationsDisallowed', window.toastDuration); }
  $('#recordP').prop('active', false);
  $('#eventP' ).prop('active', false);
  $('#adminP' ).prop('active', false);
}
