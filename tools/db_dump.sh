# backup PL database
mysqldump -v -C -c --add-drop-table --host=localhost --routines --user=root --password=*** dbNAME 1> ./dump.sql 2>> ./dump.log ;

# backup PL cache and LPF site
# tar zcvfh /pl/backup/dump.tgz /pl/tmp /pl/lpfSite 1>> /dev/null 2>> /dev/null ;
