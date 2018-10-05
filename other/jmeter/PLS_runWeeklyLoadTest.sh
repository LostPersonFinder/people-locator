## script runs from ceb-greg weekly on Sundays at 2am
cd /home/g/taupo ;
git pull ;
cd /home/g/taupo/other/jmeter ;
mkdir /home/g/taupo/other/jmeter/out ;
./apache-jmeter-3.3/bin/jmeter -n -t ./PLS_loadTest.jmx ;
mkdir /home/g/taupo/other/jmeter/report_summary ;
./apache-jmeter-3.3/bin/jmeter -g out/PLS_results_summary.csv -o ./report_summary/ ;
rm jmeter.log ;
rm *.hprof ;
rm -rf /home/g/www/report_summary ;
mv /home/g/taupo/other/jmeter/report_summary /home/g/www/report_summary ;
chmod -R 777 /home/g/www/report_summary ;
rm -rf /home/g/taupo/other/jmeter/out ;

