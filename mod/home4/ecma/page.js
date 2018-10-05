// main()
'use strict';
// define web service endpoint && locale file locations
window.endpoint = '/rest_endpoint';
I18nMsg.url = '/assets/locales';
// setup error logging
initError();
// wait for polymer to finish loading before we start execution
document.addEventListener('WebComponentsReady', function() {
  // use Polymer lazy registration feature to speed up initial boot
  window.Polymer = window.Polymer || {lazyRegister: true};
  // user group id
  window.gid = parseInt(getCookie('PL_GID'), 10);
  if(isNaN(window.gid)) { window.gid = 3; } // anon
  // load user state
  window.user = getCookie('PL_USER');
  // prefer cookie token over php token
  if(getCookie('PL_TOKEN') != null ) { window.token = getCookie('PL_TOKEN'); }
  // no cookie token so save php token
  else { setCookie('PL_TOKEN', window.token, 9999); }
  // start routing
  initRouter1();
  // hide loading throbber
  $('#beforeLoaded').css('display','none');
  // fade in header
  $('.theader').animate({ opacity: 1 }, 1000);
  // load user settings
  loadMessages();
});
