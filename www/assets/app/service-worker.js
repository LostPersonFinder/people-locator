'use strict';
var appVersion = 'v2018_0904_143645';
console.log('%cservice-worker GO!', 'color: orange');
var goto = '/';

// install app and cache resources
self.addEventListener('install', event => {
  console.log('%cservice-worker::install caching start', 'color: orange');
  event.waitUntil(
    caches.open(appVersion).then(cache => cache.addAll([
      '/',
      '/en',
      '/es',
      '/hi',
      '/ja',
      '/mm',
      '/ur',
      '/vi',
      '/zh_CN',
      '/zh_TW',
      '/manifest.json',
      '/assets/app/a.js',
      '/assets/locales/en.json',
      '/assets/locales/es.json',
      '/assets/locales/ja.json',
      '/assets/locales/ur.json',
      '/assets/locales/vi.json',
      '/assets/locales/zh_CN.json',
      '/assets/locales/zh_TW.json',
      '/assets/images/events/blue6_566.png',
      '/assets/images/events/earth7_566.jpg',
      '/assets/fonts/open-sans-v14-latin-regular.woff2',
      '/assets/fonts/open-sans-v14-latin-700.woff2',
      '/assets/icons/android-chrome-192x192.png',
      '/assets/images/logo/logo-white.svg',
      '/assets/images/logo/nih-nlm-logo-white.svg',
      '/assets/images/logo/nih-nlm-logo.svg',
      '/assets/images/logo/nih-white.svg',
      '/assets/images/other/throbber.svg',
      '/assets/images/other/clippy.svg',
      '/assets/images/other/throb_black.svg'
    ])).then( function() { console.log('%cservice-worker::install caching complete!', 'color: orange'); })
  );
});

// when activated, we purge older caches
self.addEventListener('activate', function(event) {
  console.log('%cservice-worker::activate', 'color: orange');
  event.waitUntil(
    caches.keys().then(function(cacheNames) {
      return Promise.all(
        cacheNames.filter(function(cacheName) {
          // return true if you want to remove this cache, but remember that caches are shared across the whole origin
          if(cacheName == appVersion) {
            console.log('%cservice-worker::activate preserving cache: '+cacheName, 'color: orange');
            return false;
          } else {
            console.log('%cservice-worker::activate deleting cache: '+cacheName, 'color: orange');
            return true;
          }
        }).map(function(cacheName) {
          return caches.delete(cacheName);
        })
      );
    })
  );
});

// listen for messages from app
self.addEventListener('message', function(event) {
  if(event.data.action === 'skipWaiting') {
    console.log('%cservice-worker::message skipWaiting!', 'color: orange');
    self.skipWaiting();
  }
});

// network // cache handling
self.addEventListener('fetch', function(event) {
  event.respondWith(
    caches.match(event.request).then(function(response) {
      if(response) {
        console.log('%cservice-worker::fetch CACHE: '+event.request.url, 'color: green');
        return response;
      }
      return fetch(event.request, {}).then(function(response) {
        const responseClone = response.clone();
        // rest call no cache
        if(event.request.method == 'POST') {}
        // cache all other network requests
        else {
          caches.open(appVersion).then(cache => cache.put(event.request, responseClone)).then( function() {
            console.log('%cservice-worker::fetch KACHD: '+event.request.url, 'color: lightgreen');
          })
        }
        return response;
      }).catch(function(error) {
        console.log('%cservice-worker::fetch FAIL: '+event.request.url, 'color: red');
        throw error;
        return null;
      });
    })
  );
});

// handle push notifications
self.addEventListener('push', function(event) {
  console.log('%cservice-worker::push -> received!', 'color: pink');
  var push = event.data.json();
  const title = push.notification.title;
  const options = {
    body:  push.notification.body,
    icon:  push.data.icon,
    badge: push.data.badge
  };
  goto = push.data.goto;
  event.waitUntil(self.registration.showNotification(title, options));
});

// respond to push notification click
self.addEventListener('notificationclick', function(event) {
  console.log('%cservice-worker::notificationclick; tag: '+event.notification.tag, 'color: pink');
  event.notification.close();
  // this looks to see if the current url is already open and focuses if it is
  event.waitUntil(clients.matchAll({ type: "window" }).then(function(clientList) {
    for(var i = 0; i < clientList.length; i++) {
      var client = clientList[i];
      if(client.url == '/' && 'focus' in client) return client.focus();
    }
    if(clients.openWindow) return clients.openWindow(goto);
  }));
});
