# pfmon.py
# Author: Owen Royall-Kahin (oroyallkahin@gmail.com)

# Reads off of a person finder feed, saves the data into files,
# and sends out an email if any new items are added to the feed.
# For python 2.7

# CHANGELOG:
#
# 5-30-2013 - Leif
# - Fleshed out plain text attachment for mobile devices
# 11-30-2012
# - Change ElementTree package name for python 2.6 compatibility
# 8-6-2012: v1.1 
# - Changes made for python 2.4 compatibility, no longer 2.7 compatible
#   - Exception handling, email library imports
# - Smarter parsing
# - Wrote messy code

####### Imports #######

import sys, re, shutil, smtplib, urllib, datetime
import xml.etree.ElementTree as ET
#from email.mime.multipart import MIMEMultipart
from email import MIMEMultipart
#from email.mime.text import MIMEText
from email import MIMEText


####### Constants #######

WORKING_DIR = '/opt/pl/tools/'
#WORKING_DIR = '/home/lneve/v/tools/'
LOG_DIR = '/opt/pl/www/tmp/pfmon_logs/'
CONFIG = WORKING_DIR + "pfmon.conf"
CURRENT_FEED = LOG_DIR + "current.xml"
FEED_HISTORY = LOG_DIR + "history.txt"
LOG = LOG_DIR + "pfmon.log"
COMMASPACE = ', '
ME = ""
SMTP_SERVER = "mailfwd.nih.gov"
LANGUAGE = "en"

####### Globals #######

repo_url = "" 
recipients = [] 

####### Procedures #######


def load_conf(filename):
  file = open(filename)
  for line in file:
    if line[0] != "#":
      items = line.split("=")
      if items[0] == "repo":
        global repo_url
        repo_url = items[1].strip()
      elif items[0] == "recipients":
        global recipients
        recipients = items[1].strip().split(",")
      elif items[0] == "sender":
        global ME
        ME = items[1].strip()
      elif items[0] == "language":
        global LANGUAGE
        LANGUAGE = items[1].strip()



# Timestamp log as event occurs, takes an optional message
def log(message="None"):
  
  # Initialize list to contain file lines, contains latest entry
  log_list = [str(datetime.datetime.now()) + " -Message: " +  str(message) + "\n"]
  
  try:
    # Attempt to open file
    log_file = open(LOG, "r")
    
    # Dump file contents into a list
    for line in list(log_file):
      log_list.append(line)
  
  except IOError, e: # replace comma with ' as' for newer versions of python
    if e.errno == 2:
      pass
    else:
      raise Exception("Error accessing log file for reading. Reason: ", e)
  
  try:
    # Update the log file with the new contents
    log_file = open(LOG, "w")
    log_file.writelines(log_list)
    log_file.close()
    
  except Exception, e: # replace comma with ' as' for newer versions of python
    raise Exception("Error accessing log file for writing. Reason: ", e)
    
  
def add_to_history():
  # Rotate feed file
  history_list = list(open(CURRENT_FEED))
  
  try:
    # Attempt to open history file
    history_file = open(FEED_HISTORY, "r")
    
    for line in history_file:
      history_list.append(line)
    
  except IOError, e: # replace comma with ' as' for newer versions of python
    if e.errno == 2:
      pass
    else:
      raise Exception("Error accessing history feed for reading. Reason: ", e)
  
  try:
    # Update the feed history file.
    history_file = open(FEED_HISTORY, "w")
    history_file.writelines(history_list)
    history_file.close()
  except Exception, e: # replace comma with ' as' for newer versions of python
    raise Exception("Error accessing history feed for writing. Reason: ", e)
  
# Return a set of english titles and identifiers from a feed in the form (title, key name)
# Input feeds must be in the form of an ElementTree
def get_titles(feed):
  eset = set([])
  
  # These are not needed if they are not expected to change.
  # Both the Google Schema and Atom namespace are listed at the top.
  # Get Atom XML tag 
  #ns = re.match("{.*}", feed.getroot().tag).group()

  # Get the Google Schema tag
  
  if feed != None:
      #print feed.getroot().ns_map['gpf']
      ns = "{"+feed.getroot().ns_map['']+"}"
      gns = "{"+feed.getroot().ns_map['gpf']+"}"
      
      for element in list(feed.getroot()):
        for title in element.findall(ns+'content/'+gns+'repo/'+gns+'title'):
          titlestr = title.items()
          if titlestr[0][1] == LANGUAGE:
            s = (title.text, \
    element.find(ns+'id').text.split('http://google.org/personfinder/')[1])
            eset.add(s)
            break

  # Returns this set. 
  # Note that this will ignore events without posted <LANGUAGE> titles
  return eset
  

# Takes in ???
def send_email(new_entries = [], removed_entries = []):
  
  msg = MIMEMultipart.MIMEMultipart('alternative')
  msg['To'] = COMMASPACE.join(recipients)
  msg['From'] = ME
  msg['Subject'] = "Email Notification of PF Change"
  msg.preamble = "preamble - pf has been changed"
  msg.epilogue = "epilogue"
  
  plain_new_entries_msg = ""
  if len(new_entries) != 0:
    plain_new_entries_msg = "".join("   Title: "+entry[0] + \
", Key: "+entry[1] + "\n" for entry in list(new_entries))
  else:
    plain_new_entries_msg = "   None"
  
  new_entries_msg = ""
  if len(new_entries) != 0:
    new_entries_msg = "".join("<p style=\"margin-left:30px;\">Title: "+entry[0] + \
", Key: "+entry[1] + "</p>" for entry in list(new_entries))
  else:
    new_entries_msg = "<p style=\"margin-left:30px;\">None</p>"
  
  plain_removed_entries_msg = ""
  if len(removed_entries) != 0:
    plain_removed_entries_msg = "".join("   Title: "+entry[0] + \
", Key: "+entry[1] + "\n" for entry in list(removed_entries))
  else:
    plain_removed_entries_msg = "   None"
  
  removed_entries_msg = ""
  if len(removed_entries) != 0:
    removed_entries_msg = "".join("<p style=\"margin-left:30px;\">Title: "+entry[0] + \
", Key: "+entry[1] + "</p>" for entry in list(removed_entries))
  else:
    removed_entries_msg = "<p style=\"margin-left:30px;\">None</p>"
  
  #plain = "PF has been changed."
  plain = """\
Google PF event feed has changed.
New Feeds:
""" + plain_new_entries_msg + \
"""
Removed Feeds:
""" + plain_removed_entries_msg + \
"""
"""

  html = """\
<html>
  <head></head>
  <body>
  <p>Hello,</p>
  <p>This message is to inform you that the Google PF event feed has changed.</p>
  <p>The events below are in the form (title, key name)</p>
  <p>New Feeds:
""" + new_entries_msg + \
"""
  </p><p>Removed Feeds:
""" + removed_entries_msg + \
"""</p>
  <p> This message was generated by pfmon running on NLM/CEB server lhce-plvm1-rh7.  Do not reply to this message.</p>
  </body>
</html>
"""

  # for each item in the entry's getiterator() print tag and text
  msg.attach(MIMEText.MIMEText(plain, 'plain'))
  msg.attach(MIMEText.MIMEText(html, 'html'))
  s = smtplib.SMTP(SMTP_SERVER)
  
  s.sendmail(ME, recipients, msg.as_string())
  s.close()
  


def main():
  # Runs main program loop. 
  try:
    # Load config settings
    load_conf(CONFIG)
    # get feed
    new_file_path = urllib.urlretrieve(repo_url)[0]
    new = parse_map(open(new_file_path))
    try:
        old = parse_map(open(CURRENT_FEED))
    except Exception, e:
        print "Existing file was empty"
        old = None
    new_entries = list(get_titles(new) - get_titles(old))
    removed_entries = list(get_titles(old) - get_titles(new))
    # Check for differences
    if (new_entries or removed_entries):
      # New content detected;
      # Log the event
      log(new_entries)
      
      #Rotate history files
      add_to_history()
      
      # Update 'current' definitions, currently disabled for testing purposes
      shutil.move(new_file_path, CURRENT_FEED)
      
      # Notifier options
      # Send email
      send_email(new_entries, removed_entries)
    
      # possibly update PL for a new event
      return 0
  
  except Exception, e: # Should also except url errors, expat errors
    # replace comma with ' as' for newer versions of python
    print "An error has occurred: ", e
    return 1
    
  return 0
  
# taken from http://effbot.org/zone/element-namespaces.htm
# used to get namespaces
def parse_map(file):

    events = "start", "start-ns", "end-ns"
    
    root = None
    
    ns_map = []
    
    for event, elem in ET.iterparse(file, events):
        if event == "start-ns":
            ns_map.append(elem)
        elif event == "end-ns":
            ns_map.pop()
        elif event == "start":
            if root is None:
                root = elem
            elem.ns_map = dict(ns_map)
    
    return ET.ElementTree(root)

if __name__ == '__main__':
  #exit(main())
  print main()
  exit
