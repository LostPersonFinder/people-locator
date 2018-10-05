# People Locator Installation Instructions #

## Prerequisites: ##

- MySQL >= 5.5
- PHP >= 7.1
- node.js
- Apache >= 2.2
  - mod_rewrite must be enabled and working
  - AllowOverride must be set to All in the  section

## Steps ##

1) Check the code out via git.  

2) The root of the repostory where this README.md file is located, must be at the path /opt/pl either physically or via soft link.  

3) Create a copy of another conf file for your instance:  

Copy __conf/config_greg.inc__ to __conf/config_my.inc__  

4) Set config to use:  
- edit __conf/taupo.conf__ and set the config file name `require_once('config_my.conf');`  

5) Create a copy of another htaccess file for your instance  
- Copy __conf/htaccess_greg.properties__ to __conf/htaccess_my.properties__  

6) Create a symbolic link to load the proper htaccess for your site
- execute the following commands while in the taupo/www folder in a shell:  
`cd www`  
`ln -s ../conf/htaccess_my.properties .htaccess`  

7) Setup the database  
- Create a new MySQL Database on your server  
- Assign a new user to the database  
- Edit the config file from the step 3 with the new database's settings  

8) Import a starter database from __docs/pl_starter_db.sql__ to the db you created in the previous step.  
`/usr/bin/mysql -p -u root DBNAME < /opt/pl/docs/pl_starter_db.sql`  

9) Make sure the cache is world writeable:  
`cd www`  
`chmod -R 777 tmp`  

10) Install npm dependencies:  

`npm install -g bower
npm install -g vulcanize
npm install -g html-minifier
npm install -g jscrambler
npm install -g crisper
npm install -g csso
npm install -g csso-cli
npm install -g uglify-js`

11) Build the app.  
`cd mod/home4`  
execute `./build.sh` for a complete build -or- `./quick.sh` for a quick build.  

12) The app should now load in a browser!  

13) Create a new admin account  
- first, register as a new user
- promote the new user to Admin directly in the database
- find the new user by email in the users database table and modify the gid (group_id) value for this user to 1
- logout and log in once again
- you can now access the admin section of the site /admin

## optional steps ##

### Configuring PL to use SOLR ###  

Installing SOLR in PL is only required for Visual Search. But for large events it may make PL searches run faster.  
  
To configure SOLR in PL, first install SOLR 7 or above (on the same server as PL or on another). Then move the following PL files to the conf directory of your new SOLR core:  
- /opt/pl/conf/schema.xml         (OPTIONAL: Edit this file and substitute your core name for “lpf”.)  
- /opt/pl/conf/solrconfig.xml     (REQUIRED: Edit this file and substitute your core name for “lpf” throughout.)  
- /opt/pl/conf/db-data-config.xml (REQUIRED: Edit this file and substitute your database server hostname for “pl-db”.)  
  
If you will be configuring Visual Search, move the following PL file to the etc  directory of your SOLR server:  
- /opt/pl/conf/jetty.xml  
  
Then modify [TBD - main PL configuration file] as follows:  
- $conf['SOLR_on']    = true;  
- $conf['SOLR_root']  = 'http://localhost/solr/lpf/'; // change “lpf” to match your SOLR core name  
- $conf['SOLR_port']  = 8983;                         // change to your SOLR port if not using default  
- $conf['SOLR_hosts'] = [“solr_host1”,"solr_host2"];  // change to your list of SOLR hostnames (could be just one)  
  
SOLR updates and deletions are done in real-time, as new reports are made or old records deleted. But to keep SOLR perfectly in sync with the database and its indexes optimized, it’s best to do a full reload of the SOLR indexes periodically. This can be done by running this PL script (which reloads the indexes every night at midnight) in the background:  
- /opt/pl/tools/solr_imports.pl

### Cron Jobs ###  

Report any PFIF,SOLR,FM errors to the developer. (suggested cron jobs)  
```
0 5 * * * cd /opt/pl/tools; ./checkPfifErrors.php  
0 3 * * * cd /opt/pl/tools; ./checkSolrErrors.php  
0 5 * * * cd /opt/pl/tools; ./checkFmErrors.php  
```

Periodic email reports. (suggested cron jobs)  
```
0 7 * * 1-5 cd /opt/pl/tools; ./mailAnalyticsReport.sh user@host
0 7 * * 6-7 cd /opt/pl/tools; ./mailAnalyticsReport.sh user@host,user2@host
5 7 * * MON cd /opt/pl/tools; ./mailEventsReport.sh user@host
10 7 * * MON cd /opt/pl/tools; ./mailEventsReportInternal.sh user@host
15 7 * * MON cd /opt/tp/tools; ./mailEventsReport.sh user@host
20 7 * * MON cd /opt/tp/tools; ./mailEventsReportInternal.sh user@host,user2@host
```
