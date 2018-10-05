// taupo utility lib
'use strict';

// console logging
function clog(msg, level, form) {
  if(form === undefined) { form = ''; }
  if(window.cloglevel === 0 && level <= 0) { console.log(msg, form); } else
  if(window.cloglevel === 1 && level <= 1) { console.log(msg, form); } else
  if(window.cloglevel === 2 && level <= 2) { console.log(msg, form); } else
  if(window.cloglevel === 3 && level <= 3) { console.log(msg, form); } else
  if(window.cloglevel === 4 && level <= 4) { console.log(msg, form); } else
  if(window.cloglevel === 5 && level <= 5) { console.log(msg, form); } else
  if(window.cloglevel === 6 && level <= 6) { console.log(msg, form); } else
  if(window.cloglevel === 7 && level <= 7) { console.log(msg, form); } else
  if(window.cloglevel === 8 && level <= 8) { console.log(msg, form); } else
  if(window.cloglevel === 9 && level <= 9) { console.log(msg, form); } else { return; }
}

// capture errors
function initError() {
  window.onerror = function(message, file, lineNumber, columnNumber, error) {
    try {
      var stack = '';
      // some browsers don't support error param yet
      if(error !== undefined) { stack = error.stack; }
      // capture location
      var lj = JSON.stringify(window.location);
      error2server(message, file, lineNumber, columnNumber, stack, lj);
    } catch (e) {}
  };
  window.uap = new UAParser(); // user agent parse init
}

// log errors to server
function error2server(message, file, lineNumber, columnNumber, stack, lj) {
  var url = window.baseUrl
    + '/rest_error?message='+btoa(message)
    + '&file='+btoa(file)
    + '&line='+lineNumber
    + '&column='+columnNumber
    + '&stack='+btoa(stack)
    + '&uid='+window.uid
    + '&gid='+window.gid
    + '&browser='+btoa(window.uap.getBrowser().name+' '+window.uap.getBrowser().version)
    + '&os='+btoa(window.uap.getOS().name+' '+window.uap.getOS().version)
    + '&lj='+btoa(lj);
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { clog('%clib::error2server FAILED 01!', 9, 'color: red');}
    var resp = JSON.parse(e.target.response);
    clog('%clib::error2server response:'+resp.msg, 9, 'color: red');
  }.bind(this);
  xhr.onerror = function(e) { clog('%clib::error2server FAILED 02!', 9, 'color: red'); };
  xhr.send();
}

// remind out of date browsers to upgrade after 2 seconds
function everGreen() {
  setTimeout(function() {
    clog('%clib::everGREEN', 9, 'color: grey');
    var $buoop = {vs:{i:13,f:-2,o:-2,s:9,c:-2},unsecure:true,api:4}; 
    function $buo_f(){ 
     var e = document.createElement("script"); 
     e.src = "//browser-update.org/update.min.js"; 
     document.body.appendChild(e);
    };
    try { document.addEventListener("DOMContentLoaded", $buo_f,false) }
    catch(e){ window.attachEvent("onload", $buo_f) }
  }, 2000);
}

// php str_ireplace equivalent
function str_ireplace(search, replace, subject, count) {
  var i = 0,
    j = 0,
    temp = '',
    repl = '',
    sl = 0,
    fl = 0,
    f = '',
    r = '',
    s = '',
    ra = '',
    sa = '',
    otemp = '',
    oi = '',
    ofjl = '',
    os = subject,
    osa = Object.prototype.toString.call(os) === '[object Array]';
  if(typeof(search) === 'object') {
    temp = search;
    search = new Array();
    for(i=0; i<temp.length;i+=1) { search[i] = temp[i].toLowerCase(); }
  } else { search = search.toLowerCase(); }
  if(typeof(subject) === 'object') {
    temp = subject;
    subject = new Array();
    for(i=0; i<temp.length;i+=1) { subject[i] = temp[i].toLowerCase(); }
  } else { subject = subject.toLowerCase(); }
  if(typeof(search) === 'object' && typeof(replace) === 'string' ) {
    temp = replace;
    replace = new Array();
    for (i=0; i < search.length; i+=1) { replace[i] = temp; }
  }
  temp = '';
  f = [].concat(search);
  r = [].concat(replace);
  ra = Object.prototype.toString.call(r) === '[object Array]';
  s = subject;
  sa = Object.prototype.toString.call(s) === '[object Array]';
  s = [].concat(s);
  os = [].concat(os);
  if(count) { this.window[count] = 0; }
  for (i = 0, sl = s.length; i < sl; i++) {
    if (s[i] === '') { continue; }
    for (j = 0, fl = f.length; j < fl; j++) {
      temp = s[i] + '';
      repl = ra ? (r[j] !== undefined ? r[j] : '') : r[0];
      s[i] = (temp).split(f[j]).join(repl);
      otemp = os[i] + '';
      oi = temp.indexOf(f[j]);
      ofjl = f[j].length;
      if(oi >= 0) { os[i] = (otemp).split(otemp.substr(oi,ofjl)).join(repl); }
      if (count) { this.window[count] += ((temp.split(f[j])).length - 1); }
    }
  }
  return osa ? os : os[0];
}

// php trim equivalent
function trim(str, charlist) {
  // from http://phpjs.org/functions/trim/
  var whitespace, l = 0, i = 0;
  str += '';
  // default list
  if (!charlist) { whitespace = " \n\r\t\f\x0b\xa0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u2028\u2029\u3000"; }
  // preg_quote custom list
  else {
    charlist += '';
    whitespace = charlist.replace(/([\[\]\(\)\.\?\/\*\{\}\+\$\^\:])/g, '$1');
  }
  l = str.length;
  for (i = 0; i < l; i++) { if(whitespace.indexOf(str.charAt(i)) === -1) { str = str.substring(i); break; }}
  l = str.length;
  for (i = l - 1; i >= 0; i--) { if (whitespace.indexOf(str.charAt(i)) === -1) { str = str.substring(0, i + 1); break; }}
  return whitespace.indexOf(str.charAt(0)) === -1 ? str : '';
}

// returns true on valid email false otherwise
function validateEmail(email) { 
  var re = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(email);
}

// php urlencode equivalent
function urlencode(str) {
  str = (str + '').toString();
  return encodeURIComponent(str).replace(/!/g,'%21').replace(/'/g,'%27').replace(/\(/g,'%28').replace(/\)/g,'%29').replace(/\*/g,'%2A').replace(/%20/g,'+');
}

// get cookie value
function getCookie(cname) {
  var name = cname + '=';
  var ca = document.cookie.split(';');
  for(var i=0; i<ca.length; i++) {
    var c = ca[i].trim();
    if(c.indexOf(name) === 0) { return c.substring(name.length, c.length); }
  }
  return null;
}

// set cookie value
function setCookie(cname, cvalue, exdays) {
  var d = new Date();
  d.setTime(d.getTime() + (exdays*24*60*60*1000));
  var expires = 'expires=' + d.toGMTString();
  document.cookie = cname+'='+cvalue+'; '+expires+'; path=/';
  clog('%clib::setCookie : '+cname+'='+cvalue+'; '+expires, 9, 'color: grey');
}

// retrieve
function getParameterByName(name, url) {
  if(!url) { url = window.location.href; }
  name = name.replace(/[\[\]]/g, "\\$&");
  var regex = new RegExp("[?&]"+name+"(=([^&#]*)|&|#|$)");
  var results = regex.exec(url);
  if(!results) { return null; }
  if(!results[2]) { return ''; }
  return decodeURIComponent(results[2].replace(/\+/g, " "));
}

// calculate if element is in the viewport
function inViewport($el) {
  var elH = $el.outerHeight();
  var H = $(window).height();
  var r = $el[0].getBoundingClientRect(), t=r.top, b=r.bottom;
  return Math.max(0, t>0? Math.min(elH, H-t) : (b<H?b:H));
}

// hide the on-screen keyboard after it is shown for an input field // goo.gl/VNXji2
function hideKeyboard(element) {
  element.attr('readonly', 'readonly'); // force keyboard to hide on input field
  element.attr('disabled', 'true');     // force keyboard to hide on textarea field
  setTimeout(function() {
    element.blur();  // actually close the keyboard
    element.removeAttr('readonly'); // revert
    element.removeAttr('disabled'); // revert
  }, 100);
}

// update the page title and meta description
function seoTitleMeta(path) {
  // Let i18n update.
  setTimeout(function() {
    var short = window.router.short;
    var action = window.router.action;
    var newTitle = '';
    var newMeta = '';
    if (action === 'about') {
      newTitle = localize('linkAbout')+' | NLM PEOPLE LOCATOR';
      newMeta = localize('aboutMeta')+' '+localize('aboutMeta2');
    } else if (action === 'help') {
      newTitle = localize('linkHelp')+' | NLM PEOPLE LOCATOR';
      newMeta = localize('aboutHelp')+' NLM PEOPLE LOCATOR.';
    } else if (action === 'resources') {
      newTitle = localize('linkResources')+' | NLM PEOPLE LOCATOR';
      newMeta = localize('resourceMeta')+' '+localize('aboutMeta2');
    } else if (action === 'trademark') {
      newTitle = localize('linkTrademark')+' | NLM PEOPLE LOCATOR';
      newMeta = 'NLM PEOPLE LOCATOR® '+localize('and')+' ReUnite® '+localize('trademarkText')+'.';
    } else if (action === 'follow') {
      newTitle = localize('linkFollowUs')+' | NLM PEOPLE LOCATOR';
      newMeta = localize('followUsMeta')+' NLM PEOPLE LOCATOR.';
    } else if (action === 'privacy') {
      newTitle = localize('linkPrivacy')+' | NLM PEOPLE LOCATOR';
      newMeta = localize('privacyMeta')+' NLM PEOPLE LOCATOR.';
    } else if (action === 'omb') {
      newTitle = 'OMB'+' | NLM PEOPLE LOCATOR';
      newMeta = localize('omb7submission');
    } else if (short !== '' && action === '') {
      // search
      newTitle = localize('searchTitle')+' '+getEventName(short)+' | NLM PEOPLE LOCATOR';
      newMeta = localize('searchMeta')+' '+getEventName(short)+' '+localize('searchMeta2')+' '+localize('searchMeta3')+' NLM PEOPLE LOCATOR';
    } else if (short === 'events') {
      // single event article
      newTitle = getEventName(window.singleEvent)+' | NLM PEOPLE LOCATOR';
      newMeta = localize('eventListMeta')+' '+getEventName(window.singleEvent)+' '+localize('eventListMeta3');
    } else if(typeof action == 'string' && (action.indexOf('record#') != -1)) {
      // individual record -- either search with one result or detailed view ("/view") // skip dev instance for now
      if(action.indexOf("record") == 0) {
        var rec = (action.endsWith("/view"))? action.substring(0, action.indexOf("/view")):action;
        var uuid = window.baseUuid+rec.replace('#', '.');
        var status = window.records[uuid].stat;
        if(        status ==  'mis') { status = localize('statusMissing' ).toLowerCase();
        } else if (status ==  'fnd') { status = localize('statusFound'   ).toLowerCase();
        } else if (status ==  'dec') { status = localize('statusDeceased').toLowerCase();
        } else if (status === 'inj') { status = localize('statusInjured' ).toLowerCase();
        } else if (status ==  'ali') { status = localize('statusAlive'   ).toLowerCase();
        } else {                       status = localize('statusUnknown' ).toLowerCase(); }
        newTitle = localize('recordTitle')+' '+window.records[uuid].name+' '+localize('recordTitle2')+' '+getEventName(short)+' | NLM PEOPLE LOCATOR';
        newMeta = window.records[uuid].name+' '+localize('recordMeta')+' '+status+' '+localize('recordTitle2')+' '+getEventName(short)+' '+localize('eventListMeta3')+' NLM PEOPLE LOCATOR.';
      }
    } else if (action === 'report') {
      // Report.
      newTitle = localize('reportMeta')+' '+getEventName(short)+' | NLM PEOPLE LOCATOR';
      newMeta = localize('reportMeta')+' '+getEventName(short)+' '+localize('searchMeta3')+' NLM PEOPLE LOCATOR.';
    } else {
      // List of articles.
      newTitle = 'NLM PEOPLE LOCATOR - '+localize('pageTitle');
      newMeta = localize('pageTitle')+' '+localize('eventListMeta3');
    }
    $('title').html(newTitle);
    $('meta[name=description]').attr('content', newMeta);
    clog('%clib::seoTitleMeta done!', 9, 'color: grey');
    // Notify Google Analytics.
    if(path !== '') { ga('send', 'pageview', path); }
    else { ga('send', 'pageview', window.location.pathname); }
  }, 2000);
}

// Convenience function to localize a string.
function localize(msg) {
  return document.querySelector('i18n-msg').getMsg(msg);
}

// store a uuid in localStorage to hide from search results
function uuidHide(uuid) {
  var hideRec = localStorage.getItem(uuid);
  if(hideRec == null) { hideRec = 0; }
  else { hideRec = parseInt(hideRec, 10); }
  hideRec++;
  localStorage.setItem(uuid,    (hideRec).toString(10))
  clog('%clib::uuidHide: '+(hideRec).toString(10), 9, 'color: grey');
  document.querySelector('taupo-results')._updateElements();
}

// get the full event name using its short name // localized if possible, otherwise english
function getEventName(short) {
  // find the event
  var found = false;
  for(var i = 0; i < window.eventData.length; i++) { if(window.eventData[i].short === short) { found = true; break; }}
  if(found) {
    if(window.eventData[i].names[window.router.locale]) { return window.eventData[i].names[window.router.locale]; }
    else { return window.eventData[i].names['en']; }
  } else {
    return "No Such Event";
  }
}

// sort a table by table header
function sortTable(n, t) {
  var table, rows, switching, i, x, y, shouldSwitch, dir, switchcount = 0;
  table = document.getElementById('allEventsTable');
  switching = true;
  // set the sorting direction to ascending
  dir = 'asc';
  // loop that will continue until no switching has been done
  while(switching) {
    // start by saying no switching is done
    switching = false;
    rows = table.getElementsByTagName('tr');
    // loop through all table rows except the first and last which contains table headers
    for(i = 1; i < (rows.length - 2); i++) {
      // start by saying there should be no switching
      shouldSwitch = false;
      // get the two elements you want to compare one from current row and one from the next
      x = rows[i].getElementsByTagName('td')[n];
      y = rows[i + 1].getElementsByTagName('td')[n];
      // check if the two rows should switch place based on the direction asc or desc (text)
      if(t && dir == 'asc') {
        if(x.innerHTML.toLowerCase() > y.innerHTML.toLowerCase()) {
          // if so, mark as a switch and break the loop
          shouldSwitch= true;
          break;
        }
      } else if(t && dir == 'desc') {
        if(x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) {
          // if so, mark as a switch and break the loop
          shouldSwitch= true;
          break;
        }
      }
      // check if the two rows should switch place based on the direction asc or desc (number)
      if(!t && dir == 'asc') {
        if(parseInt(x.innerHTML,10) > parseInt(y.innerHTML,10)) {
          // if so, mark as a switch and break the loop
          shouldSwitch= true;
          break;
        }
      } else if(!t && dir == 'desc') {
        if(parseInt(x.innerHTML,10) < parseInt(y.innerHTML,10)) {
          // if so, mark as a switch and break the loop
          shouldSwitch= true;
          break;
        }
      }
    }
    if(shouldSwitch) {
      // if a switch has been marked, make the switch and mark that a switch has been done
      rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);
      switching = true;
      // each time a switch is done, increase this count by 1
      switchcount ++;
    } else {
      // if no switching has been done AND the direction is asc, set the direction to desc and run the while loop again
      if(switchcount == 0 && dir == 'asc') {
        dir = 'desc';
        switching = true;
      }
    }
  }
}

function openJira() {
  var win = window.open('https://jira.nlm.nih.gov/issues/?filter=19103', '_blank');
  win.focus();
}
