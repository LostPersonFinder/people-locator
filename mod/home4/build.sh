#!/bin/sh
echo 'BUILD_pre-cleanup...' ;
rm -f /opt/pl/www/assets/app/a.html
rm -f /opt/pl/www/assets/app/a.js
rm -f /opt/pl/www/assets/app/a.css

echo 'BUILD_versioning service worker...'
sed "2s/.*/var appVersion = 'v$(date -u +%Y_%m%d_%H%M%S)';/" /opt/pl/www/assets/app/service-worker.js > /opt/pl/www/assets/app/service-worker2.js
rm /opt/pl/www/assets/app/service-worker.js
mv /opt/pl/www/assets/app/service-worker2.js /opt/pl/www/assets/app/service-worker.js
chmod 755 /opt/pl/www/assets/app/service-worker.js

echo 'BUILD_csso...'
cat /opt/pl/mod/home4/styles4.css /opt/pl/www/assets/bower_components/cropper/dist/cropper.css > /opt/pl/www/assets/app/b.css
csso --comments none --stat --input /opt/pl/www/assets/app/b.css --output /opt/pl/www/assets/app/a.css 2> /dev/null

echo 'BUILD_vulcanization...'
mv /opt/pl/mod/home4/page.html /opt/pl/mod/home4/elements /opt/pl/www
cd /opt/pl/www
vulcanize ./page.html --abspath /opt/pl/www --strip-comments --inline-script | crisper --html ./assets/app/b.html --js ./assets/app/b.js
cd /opt/pl/mod/home4
mv /opt/pl/www/page.html /opt/pl/www/elements /opt/pl/mod/home4

echo 'BUILD_css inlining...'
php /opt/pl/inc/lib_build.inc mergeCss

echo 'BUILD_js libs concatenation...'
cat /opt/pl/mod/home4/ecma/*.js > /opt/pl/www/assets/app/c.js
cat /opt/pl/www/assets/app/b.js   /opt/pl/www/assets/app/c.js > /opt/pl/www/assets/app/d.js

echo 'BUILD_minifying...'
html-minifier --html5 --use-short-doctype --case-sensitive --collapse-boolean-attributes --collapse-inline-tag-whitespace --collapse-whitespace --decode-entities --minify-css --process-conditional-comments --remove-attribute-quotes --remove-comments --remove-empty-attributes --remove-optional-tags --remove-script-type-attributes --remove-style-link-type-attributes --sort-attributes --sort-class-name --use-short-doctype /opt/pl/www/assets/app/b.html -o /opt/pl/www/assets/app/a.html

echo 'BUILD_uglifying...'
uglifyjs --compress --keep-fnames /opt/pl/www/assets/app/d.js -o /opt/pl/www/assets/app/a.js

echo 'BUILD_finalizing build...'
php /opt/pl/inc/lib_build.inc finalizeBuild

echo 'BUILD_post-cleanup...'
rm -f /opt/pl/www/assets/app/b.html
rm -f /opt/pl/www/assets/app/b.css
rm -f /opt/pl/www/assets/app/b.js
rm -f /opt/pl/www/assets/app/c.js
rm -f /opt/pl/www/assets/app/d.js
chmod -R 755 /opt/pl/www/assets/
chmod -R 755 /opt/pl/mod/home4/
echo 'BUILD_done!'
