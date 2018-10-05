## PL Load Testing
This script runs load testing on PL server using <a href="http://jmeter.apache.org">Apache JMeter</a>. The following are 
configuration parameters that should be tweaked before starting the load test. 
These varriables are defined in JMeter's 'User Defined Variables' object named 
as GLOBAL_VARIABLES. The script is tested with 3.3 version which is package in the current directory under ./apache-jmeter-3.3 since 4.0 is out now.

##### Third party plugins
The script uses some third party plugins. These are already included in the provided 3.3 copy of JMeter packaged here.

##### Server related parameters:
These parameters are server related. Adjust them according to server url, event selected, 
and login accounts etc.
1. HOST: {plstage.nlm.nih.gov}<br>
   The name of the test server to run the test against.
2. PORT: {443}<br>
   The port number of the server.
3. EVENT: {test}<br>
   The name of the event to run the test against.
4. LOGIN_USER: {jmetertester}<br>
   This is appended with 'User Counter' in Login Fragment to format
   actual login user id. The user counter is an integer from 1 to 100. 
   Initially 100 login accounts are setup with jmetertester[1..100]@ 
   as user id. 
5. PASSWORD: {<password>}<br>
   Check with project staff for current password. 
6. SCRIPT_BASE: {<DO NOT CHANGE THIS>}<br>
   This is a computed location which points to the directory where PLLoadTest.jmx is located.
   This is typically used for assigning paths to various log files. Do not change this unless 
   you know what you are doing.
   
##### Tester parameters
 These parameters are related to desired load on the server. They should be adjusted
 according to server capacity and failure threshold.
 
1. MAX_ACTIVE_USERS: {<Set to desired number>} <br>
   This represents the active user sessions (threads) sending the requests to the server.
   This along with RAMPUP_TIME_SECONDS will determine the level of load on the server.
2. RAMPUP_TIME_SECONDS: {<Set to desired number>}<br>
   The number of seconds to spawn the active user sessions. This represents how fast the 
   initial load should be ramped up. Sudden initial stress on the server could significantly 
   reduce subsequent throughput of the server.
3. LOOP_COUNT: {<Set to desired number>}<br>
   The number of repetitions each user session should cycle through. It helps to sustain the load
   on the server over a period of time.
4. HOME_HITS_PERCENTAGE: {<Set to desired number>}<br>
   Pecentage of hits going to home page. It is usually 100 percentage.
5. SEARCH_HITS_PERCENTAGE: {<Set to desired number>}<br>
   Pecentage of hits it should do text queries.
6. REPORT_HITS_PERCENTAGE: {<Set to desired number>}<br>
   Pecentage of hits it should do missing person report.
      
##### Test Login/Logout
The jmetertester user accounts are only required to cover user login activity. 
The user login is used only in reporting fragment. If you want to cover this functionality,
make sure to set LOGIN_USER and PASSWORD parameters as specified above. It is also necessary
that all hundred accounts are active and functional on the server for the script to run 
successfully. To avoid login, you should disable Login and Logout modules from the Run thread group.
