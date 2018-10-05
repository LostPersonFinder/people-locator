#!/bin/sh
echo 'QUICK_pre-cleanup...' ;
rm -f /opt/pl/www/assets/app/a.html
rm -f /opt/pl/www/assets/app/a.js
rm -f /opt/pl/www/assets/app/a.css

echo 'QUICK_versioning service worker...'
sed "2s/.*/var appVersion = 'v$(date -u +%Y_%m%d_%H%M%S)';/" /opt/pl/www/assets/app/service-worker.js > /opt/pl/www/assets/app/service-worker2.js
rm /opt/pl/www/assets/app/service-worker.js
mv /opt/pl/www/assets/app/service-worker2.js /opt/pl/www/assets/app/service-worker.js
chmod 755 /opt/pl/www/assets/app/service-worker.js

echo 'QUICK_css...'
cat /opt/pl/mod/home4/styles4.css /opt/pl/www/assets/bower_components/cropper/dist/cropper.css > /opt/pl/www/assets/app/a.css

echo 'QUICK_vulcanization...'
mv /opt/pl/mod/home4/page.html /opt/pl/mod/home4/elements /opt/pl/www
cd /opt/pl/www
vulcanize ./page.html --abspath /opt/pl/www --inline-script | crisper --html ./assets/app/b.html --js ./assets/app/b.js
cd /opt/pl/mod/home4
mv /opt/pl/www/page.html /opt/pl/www/elements /opt/pl/mod/home4

echo 'QUICK_css inlining...'
php /opt/pl/inc/lib_build.inc mergeCss

echo 'QUICK_js libs concatenation...'
cat /opt/pl/mod/home4/ecma/*.js > /opt/pl/www/assets/app/c.js
cat /opt/pl/www/assets/app/b.js   /opt/pl/www/assets/app/c.js > /opt/pl/www/assets/app/d.js

echo 'QUICK_move html/js...'
mv /opt/pl/www/assets/app/b.html /opt/pl/www/assets/app/a.html
mv /opt/pl/www/assets/app/d.js   /opt/pl/www/assets/app/a.js

echo 'QUICK_finalizing build...'
php /opt/pl/inc/lib_build.inc finalizeBuild

echo 'QUICK_post-cleanup...'
rm -f /opt/pl/www/assets/app/b.js
rm -f /opt/pl/www/assets/app/c.js
chmod -R 755 /opt/pl/www/assets/
chmod -R 755 /opt/pl/mod/home4/
echo 'QUICK_done!'
