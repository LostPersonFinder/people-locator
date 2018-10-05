'use strict';
// router

// initialize default router state
function initRouter1() {
  clog('%crouter::initRouter1', 9, 'color: grey');
  // we don't have event data yet
  if(JSON.parse(localStorage.getItem('eventData')) === null || localStorage.getItem('eventDataTime') === null) {
    clog('%crouter::initRouter1 no eventData cache; getting!', 9, 'color: grey');
    getEventData('initRouter1a');
  // already have event data
  } else {
    var cacheAge = (Math.floor(Date.now()/1000))-(parseInt(localStorage.getItem('eventDataTime'), 10));
    clog('%crouter::initRouter1 eventData cache exists; age: '+cacheAge+'s', 9, 'color: grey');
    // event data is stale // older than 15 minutes // 900 seconds // get new
    if(cacheAge > 900) { getEventData('initRouter1b'); } // cache is fresh
    else {
      clog('%crouter::initRouter1 eventData cache is fresh', 9, 'color: grey');
      initRouter2();
    }
  }
}

// callback after initial event data has downloaded
function initRouter1a(updateCache) {
  if(updateCache) {
    clog('%crouter::initRouter1a NEW cache!', 9, 'color: grey');
    initRouter2();
  } else { netConnectionError(); }
}

// callback after updated event data has downloaded
function initRouter1b(updateCache) {
  if(updateCache) {
    clog('%crouter::initRouter1b using new data', 9, 'color: grey');
    initRouter2();
  // use cache data anyway // offline
  } else {
    clog('%crouter::initRouter1b offline; using cache', 9, 'color: grey');
    initRouter2();
  }
}

// initialize default router state 2
function initRouter2() {
  clog('%crouter::initRouter2', 9, 'color: grey');
  // load local event data
  window.eventData = JSON.parse(localStorage.getItem('eventData'));
  // init vars
  window.disaster = null; // current event // array index of window.eventData
  window.router   = new Object();
  window.oldroute = new Object();
  window.directTerm = 'DEFAULT';
  window.searchTerm = 'DEFAULT';
  window.onpopstate = function(event) { backRoute(event); }; // catch and perform on back action
  // first we show events unless otherwise
  window.firstRoute = 'showEvents';
  window.firstParam = null;
  // detect lang
  window.dLang = window.navigator.userLanguage || window.navigator.language;
  window.dLang = str_ireplace('-', '_', window.dLang);
  // we don't support country specific languages yet (except chinese) so remove country specifier
  if(window.dLang.substr(0,2) !== 'zh') { window.dLang = window.dLang.substr(0,2); }
  // validate detected locale is enabled
  window.dLangSet = false;
  window.i18n_on.forEach(function(locale) { if(window.dLang == locale) { window.dLangSet = true; }});
  clog('%crouter::initRouter2 dLang:'+window.dLang+' dLangSet:'+window.dLangSet, 9, 'color: grey');
  // get locale cookie
  window.cLang = getCookie('cLang');
  // validate cookie locale
  window.cLangSet = false;
  window.i18n_on.forEach(function(locale) { if(window.cLang == locale) { window.cLangSet = true; }});
  clog('%crouter::initRouter2 cLang:'+window.cLang+' cLangSet:'+window.cLangSet, 9, 'color: grey');
  // parse url route
  var a = window.location.pathname.split('/');
  var hh = window.location.hash.split('#')[1];
  window.router.locale  = (typeof a[1] == 'undefined') ? '' : a[1];
  window.router.short   = (typeof a[2] == 'undefined') ? '' : a[2];
  window.router.action  = (typeof a[3] == 'undefined') ? '' : a[3];
  window.router.hashish = (typeof hh   == 'undefined') ? '' : hh;
  // validate url locale
  window.uLangSet = false;
  window.i18n_on.forEach(function(locale) { if(window.router.locale == locale) { window.uLangSet = true; }});
  clog('%crouter::initRouter2 uLang:'+window.router.locale+' uLangSet:'+window.uLangSet, 9, 'color: grey');
  // tell search engines not to index pages with invalid URLs
  if(!window.uLangSet && window.router.locale != '') { $('meta[name=robots]').attr('content', 'noindex,nofollow'); }
  // language decision tree // cookie has highest priority
  if(window.cLangSet == true) {
    // cookie and url local differ
    if(window.router.locale != window.cLang) {
      window.router.locale = window.cLang;
      replaceRoute();
    }
  // url has locale
  } else if(window.uLangSet) { window.postWelcome = false; } // window.router.locale already set; go to url
  // detected language
  else if(window.dLangSet) {
    window.router.locale = window.dLang;
    replaceRoute();
    window.postWelcome = true;
  // defaults
  } else {
    window.router.locale = 'en';
    window.router.short  = '';
    window.router.action = '';
    window.postWelcome = true;
  }
  // init i18n elements
  if(window.router.locale == 'ur') { window.rtl = true; } else { window.rtl = false; }
  I18nMsg.url = '/assets/locales';
  I18nMsg.lang = window.router.locale;
  Platform.performMicrotaskCheckpoint();
  // handle events filter // special case
  if(window.router.short == 'events') {
    window.deepLink  = 'doNothing';
    var doesExist = false;
    // check to see if the event exists
    window.eventData.forEach(function(event) { if(event.short == window.router.action) { doesExist = true; }});
    // filter event list to this single event
    if(doesExist) { window.singleEvent = window.router.action; }
    // tell user about retired/missing event
    else { window.deepLink  = 'eventRetired'; }
  // handle pages after page finishes loading
  } else if(window.router.short == 'pages') {
    switch(window.router.action) {
      case 'help':
        window.deepLink = 'loadPage';
        window.pageID   = 1;
        break;
      case 'about':
        window.deepLink = 'loadPage';
        window.pageID   = 2;
        break;
      case 'privacy':
        window.deepLink = 'loadPage';
        window.pageID   = 3;
        break;
      case 'resources':
        window.deepLink = 'loadPage';
        window.pageID   = 4;
        break;
      case 'follow':
        window.deepLink = 'loadPage';
        window.pageID   = 5;
        break;
      case 'trademark':
        window.deepLink = 'loadPage';
        window.pageID   = 6;
        break;
      case 'omb':
        window.deepLink = 'loadPage';
        window.pageID   = 7;
        break;
      case 'confirm':
        window.deepLink = 'confirmEmailAddress';
        // remove this page visit from history
        window.router.short  = '';
        window.router.action = '';
        window.z = getParameterByName('z');
        replaceRoute();
        break;
      case 'reset':
        window.deepLink = 'confirmResetPassword';
        // remove this page visit from history
        window.router.short  = '';
        window.router.action = '';
        window.z = getParameterByName('z');
        replaceRoute();
        break;
      default:
        window.deepLink = 'page404';
        // remove this page visit from history
        window.router.short  = '';
        window.router.action = '';
        replaceRoute();
    }
  // handle direct event access
  } else if(window.router.short !== '') {
    var i = null;
    var x = 0;
    // check to see if the event exists
    window.eventData.forEach(function(event) {
      if(event.short == window.router.short) { i = x; }
      x++;
    });
    // valid short
    if(i !== null) {
      // skip event list to search
      window.firstRoute = 'goSearch';
      window.firstParam = false;
      window.disaster   = i;
      window.skipHideEvents = true; // none to hide :)
      // deep event linking...
      if(window.router.action == 'report') {
        window.dontPush = true;
        window.deepLink = 'goReport';
      // direct record access // originated record
      } else if(window.router.action == 'record') {
        // get hash
        var hh = window.location.hash.split('#')[1]; // origin record# + /view
        var h  = hh.split('/')[0]; // origin record#
        var hv = hh.split('/')[1]; // view
        window.deepLink = 'doNothing';
        // deep link to open full record view
        if(typeof hv !== 'undefined' && hv == 'view') {
          window.view_uuid = window.baseUuid+window.router.action+'.'+h;
          clog('%crouter::initRouter2 direct uuid >> '+window.view_uuid, 9, 'color: lightgreen');
        }
        if(window.solr == 1) { window.directTerm = 'p_uuid:'+window.baseUuid+window.router.action+'.'+h; }
        else {                 window.directTerm = window.baseUuid+window.router.action+'.'+h; }
      // pfif record
      } else if(window.router.action == 'pfif') {
        // get hash
        var hh = window.location.hash.split('#')[1]; // pfif uuid + /view
        var h  = hh.split('/')[0]; // pfif uuid
        var hv = hh.split('/')[1]; // view
        h = str_ireplace('_', '/', h); // restore slash
        window.deepLink = 'doNothing';
        // deep link to open full record view
        if(typeof hv !== 'undefined' && hv == 'view') {
          window.view_uuid = h;
          clog('%crouter::initRouter2 direct uuid >> '+window.view_uuid, 9, 'color: lightgreen');
        }
        if(window.solr == 1) { window.directTerm = 'p_uuid:'+h; }
        else { window.directTerm = h; }
      // direct search term
      } else if(window.router.action !== '') {
        window.deepLink   = 'doNothing';
        window.directTerm = decodeURIComponent(window.router.action);
      // generic search for all
      } else { window.deepLink   = 'doNothing'; }
    // tell user about retired/missing event
    } else { window.deepLink  = 'eventRetired'; }
  // link must be shallow
  } else { window.deepLink  = 'doNothing'; }
  initRouter3();
}

// part 3
function initRouter3() {
  clog('%crouter::initRouter3', 9, 'color: grey');
  // ui // logged in
  if(window.gid == 1 || window.gid == 2) {
    var paperButton = document.querySelector('#user');
    Polymer.dom(paperButton).innerHTML = window.user;
  }
  // show admin link
  if(window.gid == 1) {
    $("#linkAdmin").removeClass('opacityZero');
    $(function() { $("#linkAdmin" ).click(function(e) { window.location = window.baseUrl+'/admin'; }); });
  }
  // watch for enter key presses globally
  $(document).keypress(function(e) { if(e.which == 13 && typeof window.enterAction !== 'undefined') { window[window.enterAction](); }});
  
/*  
  // start service worker
  if('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/service-worker.js', { updateViaCache: 'none' }).then(function(registration) {
      clog('%crouter::initRouter3 Service Worker Registered!', 9, 'color: orange');
      window.messaging.useServiceWorker(registration);
      notify_initialiseState();
    }).catch(function(err) { clog('%crouter::initRouter3 Service Worker Registration FAILED!!! error: '+err, 9, 'color: red'); });
  }
*/

  if('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/service-worker.js', { updateViaCache: 'none' }).then(function(registration) {
      
      clog('%crouter::initRouter3 Service Worker Registered!', 9, 'color: orange');
      window.messaging.useServiceWorker(registration);
      notify_initialiseState();
      
      registration.addEventListener('updatefound', function() {
        // a wild service worker has appeared in registration.installing !!
        window.newWorker = registration.installing;
        window.newWorker.addEventListener('statechange', function() {
          // new service worker state changed
          switch(window.newWorker.state) {
            case 'installed':
              if(navigator.serviceWorker.controller) { toastR.open(); } // there is a new service worker available
              else { clog('router::initRouter3 no new SW available >> '+window.newWorker.state, 9); }
            break;
          }
        });
      });
    }).catch(function(err) { clog('%crouter::initRouter3 Service Worker Registration FAILED!!! error: '+err, 9, 'color: red'); });
    
    
    var refreshing;
    navigator.serviceWorker.addEventListener('controllerchange', function () {
      clog('%crouter::initRouter3 Service Worker controllerchange!', 9, 'color: orange');
      if(refreshing) return;
      window.location.reload();
      refreshing = true;
    });
    
    
    
  }

  // post i18n setup
  postInitLang();
  initPads();
  beginRouting();
}

function updateApp() {
  //toastR.toggle();
  window.newWorker.postMessage({ action: 'skipWaiting' });
}

// inform user of retired deep event link
function eventRetired() {
  // Wait for lang pack to be loaded. 
  if (document.querySelector('i18n-msg').getMsg('eventRetired') === null) { setTimeout(function() { eventRetired(); }, 333); }
  else { 
    toastRed('eventRetired', window.toastDuration);
    // tell search engines not to index this
    $('meta[name=robots]').attr('content', 'noindex,nofollow');
  }
}

// the do nothing function! // empty callback
function doNothing() {}

// replace the current history state
function replaceRoute() {
  clog('%crouter::replaceRoute', 9, 'color: grey');
  var short   = '';
  var action  = '';
  var hashish = '';
  if(window.router.short != '') { short = '/'+window.router.short; }
  if(window.router.action != '') { action = '/'+window.router.action; }
  if(window.router.hashish != '') { hashish = '#'+window.router.hashish; }
  var stateObj = { thisState: window.router };
  history.replaceState(stateObj, null, window.baseUrl+'/'+window.router.locale+short+action+hashish);
}

// go home 
function goHome() {
  clog('%crouter::gohome', 9, 'color: blue');
  // save current state
  var short = window.router.short;
  // set next state
  window.router.short   = '';
  window.router.action  = '';
  window.router.hashish = '';
  // leaving event context
  if (validShort(short)) { leaveSearch(); }
  // leaving single event
  else if(short == 'events') {
    delete window.singleEvent;
    reloadArticles();
  }
  pushRoute();
}

// push a new route
function pushRoute() {
  var short  = '';
  var action = '';
  var h = window.location.hash.split('#')[1];
  var hash = '';
  // a hash is only shown when at an exact record url
  if(typeof h !== 'undefined' && h !== '' && window.router.action == 'record') { hash = '#'+h; }
  if(window.router.short != '') { short = '/'+window.router.short; }
  if(window.router.action != '') { action = '/'+window.router.action; } 
  var stateObj = { thisState: window.router };
  var path = '/'+window.router.locale+short+action+hash;
  history.pushState(stateObj, null, window.baseUrl+path);
  clog('%crouter::pushRoute route:'+window.baseUrl+path, 9, 'color: grey');
  // update page title and meta tags for SEO
  seoTitleMeta(path);
}

// pop the old route
function popRoute() {
  window.history.back();
  clog('%crouter::popRoute', 9, 'color: grey');
}

// when a back button is pressed
function backRoute(event) {
  // save current state before back
  window.oldroute.locale = window.router.locale;
  window.oldroute.short  = window.router.short;
  window.oldroute.action = window.router.action;
  window.oldroute.hv     = window.router.hv;
  // get new state after back/forward
  var a  = window.location.pathname.split('/');
  var hh = window.location.hash.split('#')[1];
  window.router.locale  = (typeof a[1] == 'undefined') ? '' : a[1];
  window.router.short   = (typeof a[2] == 'undefined') ? '' : a[2];
  window.router.action  = (typeof a[3] == 'undefined') ? '' : a[3];
  window.router.hashish = (typeof hh   == 'undefined') ? '' : hh;
  // parse hash
  var h  = (typeof hh == 'undefined') ? '' : hh.split('/')[0];
  var hv = (typeof hh == 'undefined') ? '' : hh.split('/')[1];
  window.router.hv = (typeof hv == 'undefined') ? '' : hv;
  var hash = '';
  // a hash is only shown when at an exact record url
  if(typeof h !== 'undefined' && h !== '' && (window.router.action == 'record' || window.router.action == 'pfif')) { hash = '#'+hh; }
  clog('router::backRoute back/forward: '+
    window.oldroute.locale+'/'+window.oldroute.short+'/'+window.oldroute.action+' to '+
    window.router.locale+  '/'+window.router.short+  '/'+window.router.action, 9);
  // same event context
  if(window.router.short == window.oldroute.short) {
    // forward to report
    if(window.router.action == 'report') {
      window.dontPush = true;
      goReport();
    // back from report
    } else if(window.oldroute.action == 'report') { hideReport(false); }
    // back from view
    else if(typeof window.oldroute.hv !== 'undefined' && window.oldroute.hv == 'view') { hideRecord(false); }
    // direct record search or view
    else if((window.router.action == 'record' || window.router.action == 'pfif') && hash !== '') {
      // back to a view
      if(window.router.hv != '' && window.router.hv == 'view') {
        window.view_uuid = 0;
        // foreign
        if(window.router.action == 'pfif') { loadRecord(str_ireplace('_', '/', h)); }
        // originated
        else { loadRecord(window.baseUuid+'record.'+h); }
      // direct walk // the only time this should be encountered is if directly walking record URL's via URL manipulation or search bot
      } else {
        clog('DIRECT WALK!', 9);
        if(window.solr == 1) { window.directTerm = 'p_uuid:'+window.baseUuid+'record.'+h; }
        else { window.directTerm = window.baseUuid+'record.'+h; }
        goQuery();
      }
    // back/forward from/to a direct term
    } else {
      window.directTerm = decodeURIComponent(window.router.action);
      // special case // empty string
      if(window.router.action == '') { window.directTerm = 'DEFAULT'; }
      goQuery();
    }
  }
  // leave event context to all events
  if(window.router.short == '' && validShort(window.oldroute.short)) { leaveSearch(); }
  // leave event context to single event
  if(window.router.short == 'events' && validShort(window.oldroute.short)) { leaveSearch(); }
  // return to event context from all events
  if((window.oldroute.short == '' || window.oldroute.short == 'events') && validShort(window.router.short)) {
    // find event index
    var x = 0;
    var i = 0;
    window.eventData.forEach(function(event) {
      if(event.short == window.router.short) { i = x; }
      x++;
    });
    window.disaster = i;
    goSearch(false);
  }
  // back/forward to a specific page
  if(window.router.short == 'pages') {
    switch(window.router.action) {
      case 'help':
        window.deepLink = 'loadPage';
        window.pageID   = 1;
        break;
      case 'about':
        window.deepLink = 'loadPage';
        window.pageID   = 2;
        break;
      case 'privacy':
        window.deepLink = 'loadPage';
        window.pageID   = 3;
        break;
      case 'resources':
        window.deepLink = 'loadPage';
        window.pageID   = 4;
        break;
      case 'follow':
        window.deepLink = 'loadPage';
        window.pageID   = 5;
        break;
      case 'trademark':
        window.deepLink = 'loadPage';
        window.pageID   = 6;
        break;
      case 'omb':
        window.deepLink = 'loadPage';
        window.pageID   = 7;
        break;
      default:
        // back to a non-existent page should never happen in theory since the history entry was replaced when first visited errantly, we'll be safe and do it again
        window.deepLink = 'page404';
        window.router.short  = '';
        window.router.action = '';
        replaceRoute();
    }
    // load specific page
    window[window.deepLink]();
  }
  // back from a specific page
  if(window.oldroute.short == 'pages') { hidePage(false); }
  // update history entry for newly selected language
  if(window.router.locale != getCookie('cLang')) {
    window.router.locale = getCookie('cLang');
    replaceRoute();
  }
}

// check an event short is valid
function validShort(shorty) {
  var doesExist = false;
  // check to see if the event exists
  window.eventData.forEach(function(event) { if(event.short == shorty) { doesExist = true; }});
  return doesExist;
}

function beginRouting() {
  clog('%crouter::beginRouting', 9, 'color: grey');
  // run this route first
  window[window.firstRoute](window.firstParam);
  // page load complete; follow deep link
  window[window.deepLink]();
  // mobile or desktop toast config
  toastFit();
  // invalid module access
  if(window.toastError !== '') { setTimeout(function() { toastRed(window.toastError, window.toastDuration); }, 1500); }
  // update page title and meta tags for SEO
  seoTitleMeta('');
  // pester old browsers
  everGreen();
  // default session change action
  window.postLoginRegisterAction = 'reloadArticles';
  // start ping every minute and once after 10 seconds
  setInterval(function(){ ping(); }, 60000);
  setTimeout(function() { ping(); }, 10000);
}
