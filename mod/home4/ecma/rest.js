'use strict';

// network error encountered
function netConnectionError() {
  toastRed('restError5000', window.toastDuration);
}

// keep alive // manage background cron tasks
function ping() {
  clog('%crest::ping', 9, 'color: white');
  var rp       = new Object();
  rp.call      = 'ping';
  rp.token     = window.token;
  rp.latitude  = 0;
  rp.longitude = 0;
  var rpj      = JSON.stringify(rp);
  fetch(window.endpoint, {
    headers: new Headers({'Content-type':'application/json;charset=UTF-8'}),
    method: 'POST',
    body: rpj
  // success
  }).then(function(response) {
    if(response.status !== 200) { clog('%crest::ping http error: '+response.status, 9, 'color: red'); return; }
    response.json().then(function(data) {
      window.offline = false;
      // valid token // cron jobs
      if(parseInt(data.error, 10) === 0) {
        // event data is stale older than 10 minutes (600 seconds) refresh
        var cacheAge = (Math.floor(Date.now()/1000))-(parseInt(localStorage.getItem('eventDataTime'), 10));
        if(cacheAge > 600) { getEventData(null); }
      }
      // invalid token // get new anon token
      else {
        clog('%crest::ping error: '+data.error, 9, 'color: red');
        rest_anon();
      }
    });
  // offline
  }).catch(function() { window.offline = true; });
}

// update event data
function getEventData(callback) {
  var rg   = new Object();
  rg.call  = 'events';
  rg.token = window.token;
  var rgj  = JSON.stringify(rg);
  fetch(window.endpoint, {
    headers: new Headers({'Content-type':'application/json;charset=UTF-8'}),
    method: 'POST',
    body: rgj
  }).then(function(response) {
    // error
    if(response.status !== 200) {
      clog('%crest::getEventData error: '+response.status, 9, 'color: red');
      if(callback !== null) { window[callback](false); }
      return;
    }
    // success
    response.json().then(function(obj) {
      window.eventData = obj;
      var timeNow = Math.floor(Date.now()/1000);
      localStorage.setItem('eventData', JSON.stringify(obj));
      localStorage.setItem('eventDataTime', timeNow);
      window.eventData = JSON.parse(localStorage.getItem('eventData'));
      clog('%crest::getEventData new event data saved to localStorage', 9, 'color: lightgreen');
      if(callback !== null) { window[callback](true); }
    });
  // offline
  }).catch(function() {
    window.offline = true;
    if(callback !== null) { window[callback](false); }
  });
}
/*
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) {
      clog('%crest::getEventData SERVER error!', 9, 'color: red');
      if(callback !== null) { window[callback](false); }
    }
    // success
    window.eventData = JSON.parse(e.target.response);
    var timeNow = Math.floor(Date.now()/1000);
    localStorage.setItem('eventData', e.target.response);
    localStorage.setItem('eventDataTime', timeNow);
    window.eventData = JSON.parse(localStorage.getItem('eventData'));
    clog('%crest::getEventData saved to localStorage', 9, 'color: #777');
    if(callback !== null) { window[callback](true); }
  }.bind(this);
  xhr.onerror = function(e) {
    clog('%crest::getEventData NETWORK error!', 9, 'color: red');
    if(callback !== null) { window[callback](false); }
  };
  xhr.send(rej);
*/  


// authenticate
function loginSub() {
  // update button
  $('#loginSub').addClass('opacityTwenty');
  $('#loginSub').prop('raised', false);
  $(function() { $("#loginSub").unbind('click'); });
  // skip outstanding request
  if(window.sending != true) {
    window.sending = true;
    // set params
    var rtu  = new Object();
    rtu.call = 'user';
    rtu.user = $('#loginUsername').val();
    rtu.pass = $('#loginPassword').val();
    var rtuj = JSON.stringify(rtu);
    var xhr  = new XMLHttpRequest();
    xhr.open('POST', window.endpoint, true);
    xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
    xhr.onload = function(e) {
      clog('%crest::loginSub response:'+e.target.status, 9, 'color: white');
      window.sending = false;
      if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
      var resp = JSON.parse(e.target.response);
      // success
      if(parseInt(resp.error, 10) === 0) {
        // update state
        window.token = resp.token;
        window.gid   = resp.gid;
        window.user  = rtu.user;
        setCookie('PL_TOKEN', resp.token, 9999);
        setCookie('PL_GID',   resp.gid,   9999);
        setCookie('PL_USER',  rtu.user,   9999);
        clog('%crest::loginSub success! token:'+window.token, 9, 'color: white');
        // logged in
        var paperButton = document.querySelector('#user');
        Polymer.dom(paperButton).innerHTML = window.user;
        // toast user login success
        toastYellow('notifyLogin', window.toastDuration);
        postLogin();
        // hide login
        document.querySelector('#pad1').close();
        // unregister enter key handler // re-register to query if in search
        if(window.router.short != '' && window.router.short != 'events') { window.enterAction = 'goQuery'; } 
        // no enter key action for events
        else { delete window.enterAction; }
        // reset login inputs
        $('#loginUsername').val('');
        $('#loginPassword').val('');
        // save locale preference
        rest_pref_locale();
        // Google Analytics
        ga('send', 'pageview', window.location.pathname + '/login');
      // failure
      } else {
        clog('%crest::loginSub failure! error:'+resp.error, 9, 'color: white');
        // invalid user or pass
        if(parseInt(resp.error, 10) === 1) { toastRed('restError1', window.toastDuration); }
        // user account is locked for abuse or repeated failed login attempts // fail2ban
        else if(parseInt(resp.error, 10) === 2) { toastRed('restError2', window.toastDuration); }
        // user account is inactive
        else if(parseInt(resp.error, 10) === 3) { toastRed('restError3', window.toastDuration); }
        // unknown error
        else { toastRed('restError9999', window.toastDuration); }
        // reset button
        $('#loginSub').removeClass('opacityTwenty');
        $('#loginSub').prop('raised', true);
        $(function() { $("#loginSub").click(function(e) { loginSub(); }); });
        // back to anon
        window.gid = 3;
      }
    }.bind(this);
    xhr.onerror = function(e) {
      window.sending = false;
      netConnectionError();
    };
    xhr.send(rtuj);
  }  
}

// register user and login
function regSub() {
  // catch multiple attempts
  if(typeof window.registering !== 'undefined') { return false; }
  // set params
  var rr     = new Object();
  rr.call    = 'register';
  rr.email   = $('#registerEmail').val();
  rr.pass    = $('#registerPassword').val(); 
  rr.captcha = window.gval;
  rr.token   = window.token;
  rr.locale  = window.router.locale;
  // invalid email
  if(!validateEmail(rr.email)) { toastRed('restError2001', window.toastDuration);
  // invalid pass
  } else if(!checkPass(rr.pass)) {
  // register
  } else {
    window.registering = true;
    // show registering status
    toastYellow('notifyRegistering', window.toastDuration);
    $('#registerSub').css({ opacity: 0.2 });
    $('#registerSub').prop('raised', false);
    $(function() { $("#registerSub").unbind('click'); });
    // call
    var rrj = JSON.stringify(rr);
    var xhr = new XMLHttpRequest();
    xhr.open('POST', window.endpoint, true);
    xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
    xhr.onload = function(e) {
      clog('%crest::regSub response:'+e.target.status, 9, 'color: white');
      delete window.registering;
      if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
      var resp = JSON.parse(e.target.response);
      // success
      if(parseInt(resp.error, 10) === 0) {
        clog('%crest::regSub success! token2:'+resp.token2, 9, 'color: white');
        // logged in as new user
        window.token = resp.token2;
        window.gid   = 2;
        window.user  = rr.email;
        setCookie('PL_TOKEN', resp.token2, 9999);
        setCookie('PL_GID',   2,           9999);
        setCookie('PL_USER',  rr.email,    9999);
        // registered
        $('#registerProgress').addClass('none');
        // logged in
        var paperButton = document.querySelector('#user');
        Polymer.dom(paperButton).innerHTML = window.user;
        postLogin();
        toastYellow('notifyRegister', window.toastDuration);
        hidePad1();
        document.querySelector('#pad1').close();
        // Google Analytics
        ga('send', 'pageview', window.location.pathname + '/register');
      // failure
      } else {
        clog('%crest::regSub failure! error:'+resp.error, 9, 'color: red');
        // show error
        switch(parseInt(resp.error, 10)) {
          case 2000:
            toastRed('restError2000', window.toastDuration);
            break;
          case 2001:
            toastRed('restError2001', window.toastDuration);
            break;
          case 2002:
            toastRed('restError2002', window.toastDuration);
            break;
          default:
            toastRed('restError9999', window.toastDuration);
            break;
        }
        // reset button
        $('#registerSub').css({ opacity: 1 });
        $('#registerSub').prop('raised', true);
        $(function() { $("#registerSub").click(function(e) { regSub(); }); });
      }
    }.bind(this);
    xhr.onerror = function(e) {
      delete window.registering;
      netConnectionError();
    };
    xhr.send(rrj);
  }
}

// perform user logout
function rest_logout() {
  var rl    = new Object();
  rl.token  = window.token;
  rl.call   = 'logout';
  rl.single = true; 
  var rlj   = JSON.stringify(rl);
  var xhr   = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    clog('%crest::logout response:'+e.target.status, 9, 'color: white');
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::logout success! token2:'+resp.token2, 9, 'color: grey');
      // update state
      window.token = resp.token2;
      window.gid   = 3;
      window.user  = null;
      setCookie('PL_TOKEN', resp.token2, 9999);
      setCookie('PL_GID',   3,           9999);
      setCookie('PL_USER',  null,        9999);
      // ui
      var paperButton = document.querySelector('#user');
      Polymer.dom(paperButton).innerHTML = '<i18n-msg msgid="signIn"></i18n-msg>';
      // toast user logout success
      toastYellow('notifyLogout', window.toastDuration);
      // hide login
      document.querySelector('#pad3').close();
      // unregister enter key handler
      delete window.enterAction;
      // leave search, then reload articles
      if(window.router.short != '' && window.router.short != 'events') {
        clog('%crest::logout leave search dont hide', 9, 'color: grey');
        unfocusSearch();
        $('#search-panel').removeClass('easeOutSearch');
        $('#search-panel').addClass('easeInSearch');
        delete window.enterAction;
        document.querySelector('taupo-search').hide();
        window.router.short = '';
        window.router.action = '';
        pushRoute();
        window.logged_out = true;
        // hide admin link
        $("#linkAdmin").addClass('opacityZero');
        $(function() { $("#linkAdmin" ).unbind('click'); });
      // just reload articles
      } else {
        clog('%crest::logout reloadArticles', 9, 'color: grey');
        reloadArticles();
      }
      window.postLoginRegisterAction = 'reloadArticles';
    // failure // invalid token // convert to anon
    } else {
      rest_anon();
      // hide login
      document.querySelector('#pad3').close();
      // unregister enter key handler
      delete window.enterAction;
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rlj); 
}

// forgot pass retrieval
function forgotSub() {
  // catch multiple attempts
  if((typeof window.forgeting !== 'undefined') && window.forgeting === true) { return false; }
  var email = $('#forgotEmail').prop('value');
  if(!validateEmail(email)) { toastRed('restError2001', window.toastDuration); return; }
  window.forgeting = true;
  // update button
  $('#forgotSub').addClass('opacityTwenty');
  $('#forgotSub').prop('raised', false);
  $(function() { $("#forgotSub").unbind('click'); });
  // call web service
  var rf   = new Object();
  rf.token = window.token;
  rf.call  = 'forgot';
  rf.email = email; 
  var rfj  = JSON.stringify(rf);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::forgotSub success!', 9, 'color: white');
      // emailed
      toastYellow('notifyResetPassword', window.toastDuration);
      hidePad1();
      document.querySelector('#pad1').close();
      setTimeout(function() { 
        // reset button
        $('#forgotSub').removeClass('opacityTwenty');
        $('#forgotSub').prop('raised', true);
        $(function() { $("#forgotSub").click(function(e) { forgotSub(); }); });
        // reset input
        $('#forgotEmail').val('');
      }, 500);
    // failure
    } else {
      // reset button
      $('#forgotSub').removeClass('opacityTwenty');
      $('#forgotSub').prop('raised', true);
      $(function() { $("#forgotSub").click(function(e) { forgotSub(); }); });
      // show error
      switch(parseInt(resp.error, 10)) {
        case 9000:
          toastRed('restError1', window.toastDuration);
          break;
        case 2001:
          toastRed('restError2001', window.toastDuration);
          break;
        default:
          toastRed('restError9999', window.toastDuration);
          break;
      }
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rfj);
  delete window.forgeting;
}

// change pass
function passwordSub() {
  // catch multiple attempts
  if((typeof window.passwording !== 'undefined') && window.passwording === true) { return; }
  var passOld  = $('#passwordOld' ).val(); 
  var passNew1 = $('#passwordNew1').val(); 
  var passNew2 = $('#passwordNew2').val(); 
  if(passNew1 != passNew2) { toastRed('restError2004', window.toastDuration); return; }
  window.passwording = true;
  // update button
  $('#passwordSub').css({ opacity: 0.2 });
  $('#passwordSub').prop('raised', false);
  $(function() { $("#passwordSub").unbind('click'); });
  // call web service
  var rp   = new Object();
  rp.token = window.token;
  rp.call  = 'change';
  rp.user  = window.user;
  rp.old   = passOld;
  rp.new   = passNew1;
  var rpj  = JSON.stringify(rp);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    clog('%crest::passwordSub response:'+e.target.status, 9, 'color: white');
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::passwordSub success!', 9, 'color: white');
      toastYellow('notifyPasswordChanged', window.toastDuration);
      hidePad3();
      document.querySelector('#pad3').close();
      setTimeout(function() {
        // reset button
        $('#passwordSub').animate({ opacity: '1' }, 500);
        $('#passwordSub').prop('raised', true);
        // reset form
        $('#passwordOld').val('');
        $('#passwordNew1').val('');
        $('#passwordNew2').val('');
      }, 500);
    // failure
    } else {
      clog('%crest::passwordSub failure! error:'+resp.error, 9, 'color: red');
      // show error
      switch(parseInt(resp.error, 10)) {
        case 1:
          toastRed('restError1', window.toastDuration);
          break;
        case 2:
          toastRed('restError2', window.toastDuration);
          break;
        case 2002:
          toastRed('restError2002', window.toastDuration);
          break;
        default:
          toastRed('restError9999', window.toastDuration);
          break;
      }
      // reset button
      $('#passwordSub').css({ opacity: 1 });
      $('#passwordSub').prop('raised', true);
      $(function() { $("#passwordSub").click(function(e) { passwordSub(); }); });
    }
    delete window.passwording;
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rpj);
}

// deep link to confirm password reset
function confirmResetPassword() {
  // call web service
  var rr   = new Object();
  rr.token = window.token;
  rr.call  = 'reset';
  rr.confirmation = window.z;
  var rrj  = JSON.stringify(rr);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success // valid confirmation link // user password reset // email sent
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::confirmResetPassword success!', 9, 'color: white');
      toastYellow('userPasswordTwo', window.toastDuration*2);
    // failure
    } else {
      clog('%crest::confirmResetPassword failure! error:'+resp.error, 9, 'color: red');
      toastRed('userConfirmInvalid', window.toastDuration*2);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rrj);
}

// deep link to confirm user email address
function confirmEmailAddress() {
  // call web service
  var rc   = new Object();
  rc.token = window.token;
  rc.call  = 'confirm';
  rc.confirmation = window.z;
  var rcj  = JSON.stringify(rc);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success // valid confirmation link // user activated
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::confirmEmailAddress success!', 9, 'color: white');
      toastYellow('userConfirmActive', window.toastDuration*2);
    // failure
    } else {
      clog('%crest::confirmEmailAddress failure! error:'+resp.error, 9, 'color: red');
      toastRed('userConfirmInvalid', window.toastDuration*2);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rcj);
}

// updates the following list
function showFollowing() {
  var html  = '';
  var rf    = new Object();
  rf.call   = 'following';
  rf.token  = window.token;
  rf.locale = window.router.locale;
  var rfj   = JSON.stringify(rf);
  var xhr   = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    $('#p3slider1').fadeTo(300,0);
    var fList = JSON.parse(e.target.response);
    var html = '<div id="followList"><table>';
    var count = 0;
    for(var i=0; i < fList.length; i++) {
      var c = new Object();
      c.uuid = fList[i].uuid;
      c.name = fList[i].name;
      if(c.name == 'unk' || c.name == '') { c.name = '<i18n-msg msgid="statusUnknown"></i18n-msg>'; }
      c.url  = fList[i].url;
      html = html +
      '<tr class="flr" id="tr'+count+'">'+
        '<td><a href="'+c.url+'">'+
          '<paper-button id="pr'+count+'" class="hoverF bgF nonUpper" role="button" tabindex="0">'+c.name+'</paper-button></a>'+
        '</td>'+
        '<td class="alignRight">'+
          '<paper-button id="uf'+count+'" onclick="unFollow('+count+', \''+c.uuid+'\');" class="hover nonUpper bgR" role="button" tabindex="0">'+
            '<i18n-msg msgid="pad3unFollow"></i18n-msg>'+
          '</paper-button>'+
        '</td>'+
      '</tr>'+
      '<tr><td class="flrr" colspan=2></td></tr>';
      count++;
    }
    if(count == 0) { html = html + '<tr><td colspan=2 class="followNone"><i18n-msg msgid="pad3emptyFollow"></i18n-msg></td></tr>'; }
    html = html + '</table></div>';
    setTimeout(function() { 
      $('#p3slider1').html(html);
      $('#p3slider1').fadeTo(300,1);
    }, 310);
  }.bind(this);
  xhr.onerror = function(e) { toastRed('restError9999', window.toastDuration); };
  xhr.send(rfj);
}

// follow/unfollow a record
function follow(uuid, sub) {
  // call web service
  var rf   = new Object();
  rf.token = window.token;
  rf.call  = 'follow';
  rf.uuid  = uuid;
  rf.sub   = sub;
  var rfj  = JSON.stringify(rf);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::follow success!', 9, 'color: white');
      if(sub) { toastYellow('notifyRecordFollow', window.toastDuration); }
      else { toastYellow('notifyRecordUnfollow', window.toastDuration); }
    // failure
    } else {
      clog('%crest::follow response: '+resp.error, 9, 'color: red');
      toastRed('restError9999', window.toastDuration);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rfj);
  ga('send', 'pageview', window.location.pathname + '/followrecord');
}

// unfollow record from follow list
function unFollow(count, uuid) {
  $('#uf'+count).fadeTo(300,0.2);
  // call web service
  var rf   = new Object();
  rf.token = window.token;
  rf.call  = 'follow';
  rf.uuid  = uuid;
  rf.sub   = false;
  var rfj  = JSON.stringify(rf);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) { netConnectionError(); return; }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::unFollow success!', 9, 'color: white');
      toastYellow('notifyRecordUnfollow', window.toastDuration);
      $('#pr'+count).fadeTo(300,0);
      $('#uf'+count).fadeTo(300,0);
      setTimeout(function() { $('#tr'+count).closest('tr').children('td').animate({ padding: 0 }).wrapInner('<div />').children().slideUp(function() { $(this).closest('tr').remove(); }); }, 310);
    // failure
    } else {
      clog('%crest::unFollow failure! error:'+resp.error, 9, 'color: red');
      toastRed('restError9999', window.toastDuration);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rfj);
}

// add comment
function addCommentF(comm) {
  // call web service
  var rc   = new Object();
  rc.token = window.token;
  rc.call  = 'comment';
  rc.uuid  = window.uuid;
  rc.text  = comm;
  rc.stat  = null;
  rc.photo = null;
  rc.latitude  = 0;
  rc.longitude = 0;
  var rcj  = JSON.stringify(rc);
  var xhr  = new XMLHttpRequest();
  xhr.open('POST', window.endpoint, true);
  xhr.setRequestHeader('Content-type', 'application/json;charset=UTF-8');
  xhr.onload = function(e) {
    if(e.target.status !== 200 && e.target.status !== 0) {
      clearSendingToastComment2();
      netConnectionError();
      return;
    }
    var resp = JSON.parse(e.target.response);
    // success
    if(parseInt(resp.error, 10) === 0) {
      clog('%crest::addCommentF success!', 9, 'color: white');
      clearSendingToastComment();
      // close add comment
      $('.commDiv').fadeTo(300,0);
      setTimeout(function() { 
        $('.commDiv').animate({ height: 0 }).animate({ padding: 0 }).css('display', 'none');
      }, 310);
      // show new comment
      var newDate0 = new Date();
      var newDate1 = newDate0.toDateString()+' '+newDate0.toLocaleTimeString();
      $('.newComm1').html(comm);
      $('.newComm2').html(newDate1);
      $('#newComm0').removeClass('none');
      // hide empty comment
      $('#blankComment').addClass('none');
      // add to local dom for caching
      window.records[window.uuid].comments.push({comment:comm, when:newDate0});
    // failure
    } else {
      clog('%crest::addCommentF failure! error:'+resp.error, 9, 'color: red');
      clearSendingToastComment2();
      toastRed('restError9999', window.toastDuration);
    }
  }.bind(this);
  xhr.onerror = function(e) { netConnectionError(); };
  xhr.send(rcj);
}

// turn session into anonymous
function rest_anon() {
  // toast logout afterwards if currently logged in
  var doToast = true;
  if(window.gid == 3) { doToast = false; }
  // get new anon token
  var ra  = new Object();
  ra.call = 'anon';
  var raj = JSON.stringify(ra);
  fetch(window.endpoint, {
    headers: new Headers({'Content-type':'application/json;charset=UTF-8'}),
    method: 'POST',
    body: raj
  // success
  }).then(function(response) {
    if(response.status !== 200) { return; }
    response.json().then(function(data) {
      // success
      if(parseInt(data.error, 10) === 0) {
        var newToken = String(data.token);
        clog('%crest::anon success!', 9, 'color: white');
        // update state
        window.token = newToken;
        window.gid   = 3;
        window.user  = null;
        setCookie('PL_TOKEN', newToken, 9999);
        setCookie('PL_GID',   3,        9999);
        setCookie('PL_USER',  null,     9999);
        // ui
        var paperButton = document.querySelector('#user');
        Polymer.dom(paperButton).innerHTML = '<i18n-msg msgid="signIn"></i18n-msg>';
        // toast user logout success
        if(doToast) { toastYellow('notifyLogout', window.toastDuration); }
        return;
      // fail2ban
      } else { clog('%crest::anon error: '+data.error, 9, 'color: red'); return; }
    });
  // offline
  }).catch(function() { window.offline = true; });
  return;
}
