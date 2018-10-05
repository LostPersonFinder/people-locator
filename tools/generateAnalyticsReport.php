#!/usr/bin/php

<?php
/**
 * Generate Google Analytics reports in html format for daily e-mail.
 * Note: Mobile data is reported to GA as "screenviews" (not pageviews).  So wherever
 * we report pageviews, these are actually screenviews+pageviews.
 */

$analyticsDir = '../3rd/analytics/';

// Load Google client libraries.
require_once ($analyticsDir . 'autoload.php');

// Set the GA profile ID  (see View->Settings).
$profileId = 'ga:1234';

// GA service account information (for lostpersonfinder@gmail.com).
$client_id = '123.apps.googleusercontent.com'; 
$service_account_name = '123@developer.gserviceaccount.com';
$key_file_location = $analyticsDir . 'ga-XXX.p12'; 

$client = new Google_Client();
$client->setApplicationName("GA Reports for PL/TT");

// Authenticate/authorize with GA.
$key = file_get_contents($key_file_location);
$cred = new Google_Auth_AssertionCredentials(
    $service_account_name,
    array('https://www.googleapis.com/auth/analytics.readonly'),
    $key
);
$client->setAssertionCredentials($cred);
$client->setClientId($client_id); 
$client->setAccessType('offline_access');

// Report period.
$period = 10;


echo "<html>";
echo "<head>";
echo "<style>";
echo 'table {max-width:600px; width:600px; border:2px solid black; border-collapse:collapse;}';
echo 'tr,td {text-align: center; border:1px solid black;}';
echo 'td {padding-left:5px; padding-right:5px;}';
echo '#title {font-weight: bold;}';
echo '@media print {#pagetop {page-break-before:always;}}';
echo "</style>";
echo "</head>";
echo "<body>";
echo '<table style="border-style:solid;border-width:1px;max-width:600px;">';
echo '<tr style="background-color:LightGreen">';
echo '<td colspan="6">';
echo '<span id="title">GOOGLE ANALYTICS REPORT FOR NLM PEOPLE LOCATOR<sup>&reg;</sup></span>';
echo '<br>All reports show both website and mobile traffic combined unless stated otherwise.';
echo '<br>View (or print) this report <a href="https://plstage.nlm.nih.gov/tmp/ga_cache/report.html">in your browser</a>.';
echo '</td>';
echo '</tr>';

// Report broken down by day.
reportDaily();
// Report broken down by event.
reportEvents();
// Report landing page host.
reportLandingPageHost();
// Report mobile usage.
reportMobile();
// Chrome doesn't seem to honor pagebreaks in tables (use divs).
echo '<tr id="pagetop" style="border:none"></tr>';
// Report broken down by source.
reportSources();
// Report broken down by social media site.
reportSocialMedia();
// Report broken down by language.
reportLanguages();
// Report broken down by location.
reportLocations();
// Report feed usage.
//reportFeeds();
//echo "\n";

echo '</table>';
echo '</body>';
echo '</html>';

// Report landing page host (lpf vs pl).
function reportLandingPageHost() {
	global $client, $profileId, $period;

	// Date range.
	$start = date("Y-m-d", strtotime("now -$period days"));
	$end = date("Y-m-d", strtotime("now"));

  echo '</tr>';
  echo '<tr>';
  echo '<td colspan="6" style="background-color:PowderBlue">
        <span id="title">WEBSITE LANDING PAGE FOR LAST '.$period.' DAYS</span>
        </td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td colspan="3">PL.NLM.NIH.GOV</td><td colspan="3">LPF.NLM.NIH.GOV</td>';
  echo '</tr>';

	// Get all path data.
	$metrics = "ga:sessions";
	$dimensions = "ga:landingPagePath,ga:hostname";
	$optParams = array('dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
	  $results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
                     $metrics,
                     $optParams);
    $counts = array('pl.nlm.nih.gov'=>0,'lpf.nlm.nih.gov'=>0);
	  if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
				$host = $row[1];
				$count = $row[2];
        // Ignore '(not set)' values from mobile.
        if ($host == 'pl.nlm.nih.gov' || $host == 'lpf.nlm.nih.gov') {
          $counts[$host] += $count;
        }
      }
    }
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
  echo '<tr>';
	echo '<td colspan="3">'.$counts['pl.nlm.nih.gov'].'</td><td colspan="3">'.$counts['lpf.nlm.nih.gov'].'</td>';
  echo '</tr>';

}

// Report feed usage by event and domain.
function reportFeeds() {
	global $client, $profileId, $period;

	// Date range.
	$start = date("Y-m-d", strtotime("now -$period days"));
	$end = date("Y-m-d", strtotime("now"));

   $reportData = array();
   $events = getEvents();

	// Max report length.
	$max = 20;

 	echo sprintf("%42s", "TOP $max FEEDS FOR LAST $period DAYS\n");
	echo sprintf("| %-20s | %-25s | %-10s |\n", 'EVENT', 'SOURCE', 'HITS');

	// Get all path data.
	$metrics = "ga:pageviews";
	$dimensions = "ga:pagePath,ga:networkDomain";
	$optParams = array('dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
                     $metrics,
                     $optParams);
		if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
				$path = $row[0];
				$domain = $row[1];
				// Look for the string "feed".
				if (!preg_match('/\/feed/', $path)) continue;
				// Get first segment of path if there is one.
				if (!preg_match('/^\/(.+)\//', $path, $matches)) continue;
				// We have a possible event.
				$event = $matches[1];
				if (in_array($event, $events)) {
					// It's a verified event so record the domain and count for the event.
         		if (array_key_exists($event, $reportData)) {
						if (array_key_exists($domain, $reportData[$event])) {
							$reportData[$event][$domain] += $row[2];
						} else {
							// Add domain to event.
							$reportData[$event][$domain] = $row[2];
						}
					} else {
						$reportData[$event] = array();
						$reportData[$event][$domain] = $row[2];
					}
				}
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
	// Sort by count.
	// First turn our array of rows into an array of columns.
	$counts = array();
	$domains = array();
	$events = array();
	foreach ($reportData as $event=>$data) {
		foreach ($data as $domain=>$count) {
			$counts[] = $count;
			$domains[] = $domain; 
			$events[] = $event;
		}
	}
	array_multisort($counts, SORT_DESC, $domains, $events);
	foreach ($counts as $index=>$count) {
		echo sprintf("| %-20s | %-25s | %-10s |\n", $events[$index], $domains[$index], $count);
	}
}
	
// Report mobile usage by os and date.
function reportMobile() {
	global $client, $profileId, $period;

	// Max report length.
	$max = 10;

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">
        <span id="title">REUNITE AND WEBSITE VISITS FOR LAST '.$period.' DAYS</span>';
  echo '</td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td>DATE</td><td>REUNITE/IOS</td><td>REUNITE/ANDROID</td><td>MOBILE/WEBSITE</td><td>TABLET/WEBSITE</td><td>DESKTOP/WEBSITE</td>';
  echo '</tr>';

	// Get all path data.
	$metrics = "ga:sessions";
	$dimensions = "ga:appName,ga:operatingSystem";
	$optParams = array('dimensions' => $dimensions,
							 'sort' => '-ga:sessions');
  $iVisitsTotal = $aVisitsTotal = $iVisitsTotal = $mVisitsTotal = $tVisitsTotal = $wVisitsTotal = 0;

	// Show sessions for past $period days.
	for ($i=1; $i <= $max; ++$i) {
		$begin = strtotime("-".$i." days");
		$end = $start = date("Y-m-d", $begin);

		$service = new Google_Service_Analytics($client);
		try {
			$results = $service->data_ga->get($profileId,
                        $start,
                        $end, 
								        $metrics,
								        $optParams);
      // Get breakdown of app sessions.
			if (!empty($results->rows)) {
				$iVisits = $aVisits = 0;
				foreach ($results->rows as $row) {
					if (strcasecmp($row[1], "ios") === 0) {
						$iVisits = $row[2];	
					} elseif (strcasecmp($row[1], "android") === 0) {
						$aVisits = $row[2];
					}
				}

	      $dimensions2 = "ga:deviceCategory,ga:browser";
	      $optParams2 = array('dimensions' => $dimensions2,
							 'sort' => '-ga:sessions');
        // Get breakdown of non-app sessions by device category.
        // Near as I can figure, app sessions come from the GoogleAnalytics browser. So filter 'em out.
			  $results = $service->data_ga->get($profileId,
                        $start,
                        $end, 
								        $metrics,
                        $optParams2);
			  if (!empty($results->rows)) {
          $mVisits = $tVisits = $wVisits = 0;
				  foreach ($results->rows as $row) {
					  if (strcasecmp($row[1], "GoogleAnalytics") === 0) continue;
					  if (strcasecmp($row[0], "desktop") === 0) {
						  $wVisits += $row[2];	
					  } elseif (strcasecmp($row[0], "tablet") === 0) {
						  $tVisits += $row[2];
					  } elseif (strcasecmp($row[0], "mobile") === 0) {
						  $mVisits += $row[2];
            }
          }
        }

        echo '<tr>';
				echo '<td>'.date(" Y/m/d ", $begin).'</td><td>'.$iVisits.'</td><td>'.$aVisits.'</td><td>'.$mVisits.'</td><td>'.$tVisits.'</td><td>'.$wVisits.'</td>';
        echo '</tr>';
        
        $iVisitsTotal += $iVisits;
        $aVisitsTotal += $aVisits;
        $mVisitsTotal += $mVisits;
        $tVisitsTotal += $tVisits;
        $wVisitsTotal += $wVisits;
			}
		} catch(Exception $e) {
   		echo 'There was an error : - ' . $e->getMessage();
		}
	}
  echo '<tr>';
	echo '<td><b>Total</b></td><td><b>'.$iVisitsTotal.'</b></td><td><b>'.$aVisitsTotal.'</b></td><td><b>'.$mVisitsTotal.'</b></td><td><b>'.$tVisitsTotal.'</b></td><td><b>'.$wVisitsTotal.'</b></td></b>';
  echo '</tr>';
}
	
// Report social media usage for last 10 days.
function reportSocialMedia() {
	global $client, $profileId, $period;

	// Max report length.
	$max = 20;

	// Date range
	$start = date("Y-m-d", strtotime("yesterday -$period days"));
	$end = date("Y-m-d", strtotime("yesterday"));

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">
        <span id="title">SOCIAL MEDIA REFERRALS TO WEBSITE FOR LAST '.$period.' DAYS</span>
        <br>Only social media sites with non-zero visits are shown.
        </td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td colspan="2">CHANNEL</td><td colspan="2">VISITS</td><td colspan="2">PAGEVIEWS</td>';
  echo '</tr>';

	$metrics = "ga:sessions,ga:pageviews";
	$dimensions = "ga:socialNetwork";
	$optParams = array('max-results' => $max,
							 'sort' => '-ga:sessions,-ga:pageviews',
							 'dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
							$metrics,
							$optParams);
		if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
        if ($row[0] == '(not set)') continue;
        echo '<tr>';
				echo '<td colspan="2">'.$row[0].'</td><td colspan="2">'.$row[1].'</td><td colspan="2">'.$row[2].'</td>';
        echo '</tr>';
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
}
	
// Show recent daily sessions.
function reportDaily() {
	global $client, $profileId;

  $userTotal = $sessionTotal = $pageviewTotal = 0;

	// Max report length.
	$max = 10;

  echo '<tr style="background-color:PowderBlue;">';
  echo '<td colspan="6">
 	      <span id="title">ALL ACTIVITY LAST '.$max.' DAYS</span>
        <br>BOUNCE RATE is the percentage of visitors that leave after just one pageview.
        </td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td>DATE</td><td>USERS</td><td>VISITS</td><td>PAGEVIEWS</td><td>AVG VISIT DURATION</td><td>BOUNCE RATE</td>';
  echo '</tr>';

	// Show sessions for past $period days.
	for ($i=1; $i <= $max; ++$i) {
		$begin = strtotime("-".$i." days");
		$end = $start = date("Y-m-d", $begin);
		$metrics = "ga:users,ga:sessions,ga:pageviews,ga:screenviews,ga:avgSessionDuration,ga:bouncerate";

		$service = new Google_Service_Analytics($client);
		try {
			$results = $service->data_ga->get($profileId,
                    $start,
                    $end, 
								    $metrics);
			$totals = $results->totalsForAllResults;
			$bouncerate = (int)$totals['ga:bouncerate'] . "%";
			$pageviews = $totals['ga:pageviews'] + $totals['ga:screenviews'];
      $avgDuration = (int)$totals['ga:avgSessionDuration'] . " secs";
      echo '<tr>';
			echo '<td>'.date("Y/m/d", $begin).'</td><td>'.$totals['ga:users'].'</td><td>'.$totals['ga:sessions'].'</td><td>'.$pageviews.'</td><td>'.$avgDuration.'</td><td>'.$bouncerate.'</td>';
		} catch(Exception $e) {
   		echo 'There was an error : - ' . $e->getMessage();
		}
    $userTotal += $totals['ga:users'];
    $sessionTotal += $totals['ga:sessions'];
    $pageviewTotal += $pageviews;
	}
  echo '<tr>';
	echo '<td><b>Total</b></td><td><b>'.$userTotal.'</b></td><td><b>'.$sessionTotal.'</b></td><td><b>'.$pageviewTotal.'</b></td><td></td><td></td>';
  echo '</tr>';
}

function reportEvents() {
	global $client, $profileId, $period;

	// Date range.
	$start = date("Y-m-d", strtotime("yesterday -$period days"));
	$end = date("Y-m-d", strtotime("yesterday"));

  $reportData = array();
  $events = getEvents();
  $shorts = array();
  $longs = array();
  foreach ($events as $event) {
    $shorts[] = $event["short"];
    $longs[$event["short"]] = $event["names"]["en"];
  }

	// Max 
	$max = 20;

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">
        <span id="title">ACTIVITY FOR LAST '.$period.' DAYS FOR OPEN EVENTS</span>
        </td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td colspan="3">EVENT</td><td colspan="3">PAGEVIEWS</td>';
  echo '</tr>';

	// Get event data from mobile sources.
  $metrics = "ga:screenviews";
	$dimensions = "ga:screenName";
	$optParams = array('sort' => '-'.$metrics,
			 				'dimensions' => $dimensions);

  $service = new Google_Service_Analytics($client);
  try {
    $results = $service->data_ga->get($profileId,
              $start,
              $end,
              $metrics,
              $optParams);
    if (!empty($results->rows)) {
      foreach ($results->rows as $row) {
        if (preg_match("/\((.+)\)/", $row[0], $match)) {
          $short = $match[1];
				  if (in_array($short, $shorts)) {
					  if (array_key_exists($short, $reportData)) {
					    $reportData[$short] += $row[1];
            } else {
					    $reportData[$short] = $row[1];
            }
				  } else {
            //echo $short.'<br>';
          }
        }
      }
    }
  } catch(Exception $e) {
    echo 'There was an error : - ' . $e->getMessage();
  }

	// Get event data from non-mobile sources by looking in the path.
	$metrics = "ga:pageviews";
	$dimensions = "ga:pagePath";
	$optParams = array('sort' => '-'.$metrics,
			 				'dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
              $start,
              $end, 
							$metrics,
							$optParams);
		if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
				$path = $row[0];
        // Look for event in path.
        // Since an event name could subsumed by another event name, we need to anchor name in search.
        foreach ($shorts as $short) {
          if (strpos($path, '/'.$short.'/') !== false || endsWith($path, '/'.$short)) {
         		if (array_key_exists($short, $reportData)) {
						  $reportData[$short] += $row[1];
					  } else {
						  $reportData[$short] = $row[1];
					  }
          }
        }
/*
				// Ignore paths not in list of PL events.
				if (in_array($path, $shorts)) {
         		if (array_key_exists($path, $reportData)) {
						$reportData[$path] += $row[1];
					} else {
						$reportData[$path] = $row[1];
					}
				} else {
          //echo $path . '<br>';
        }
*/
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}

	arsort($reportData);
	foreach ($reportData as $event=>$count) {
    echo "<tr>";
		echo '<td colspan="3">'.$longs[$event].'</td><td colspan="3">'.$count.'</td>';
    echo "</tr>";
	}
}
	
	
function reportSources() {
	global $client, $profileId, $period;

	// Max report length.
	$max = 10;

	// Date range
	$start = date("Y-m-d", strtotime("yesterday -$period days"));
	$end = date("Y-m-d", strtotime("yesterday"));

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">';
  echo '<span id="title">TOP '.$max.' WEBSITE SOURCES FOR LAST '.$period.' DAYS</span>';
  echo '<br>A medium of "(none)" means traffic originated from our URL having been entered in a web browser.';
  echo '<br>A medium of "referral" means traffic originated from a link on the specified website.';
  echo '<br>A medium of "organic" means traffic originated from a search on the specified website.';
  echo '</td>';
  echo '</tr>';
  echo '<tr>';
  echo '<td colspan="2">SOURCE</td><td>MEDIUM</td><td>VISITS</td><td>PAGEVIEWS</td><td>BOUNCE RATE</td>';
  echo '</tr>';

	$metrics = "ga:sessions,ga:pageviews,ga:bouncerate";
	$dimensions = "ga:source,ga:medium";
	$optParams = array('max-results' => $max,
							 'sort' => '-ga:sessions,-ga:pageviews',
							 'dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
							$metrics,
							$optParams);
		if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
				$bouncerate = (int)$row[4] . "%";
        echo '<tr>';
				echo '<td colspan="2">'.$row[0].'</td><td>'.$row[1].'</td><td>'.$row[2].'</td><td>'.$row[3].'</td><td>'.$bouncerate.'</td>';
        echo '</tr>';
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
}
	
// Language report.
function reportLanguages() {
	global $client, $profileId, $period;

	// Max report length.
	$max = 10;

	// Date range
	$start = date("Y-m-d", strtotime("yesterday -$period days"));
	$end = date("Y-m-d", strtotime("yesterday"));

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">
        <span id="title">TOP '.$max.' LANGUAGES FOR LAST '.$period.' DAYS</span>
        </td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td colspan="2">LANGUAGE</td><td colspan="2">VISITS</td><td colspan="2">PAGEVIEWS</td>';
  echo '</tr>';

	$metrics = "ga:sessions,ga:pageviews,ga:screenviews";
	$dimensions = "ga:language";
	$optParams = array('max-results' => $max,
							 'sort' => '-ga:sessions',
							 'dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
							$metrics,
							$optParams);
		if (!empty($results->rows)) {
      $langs = array();
			foreach ($results->rows as $row) {
        // Remove regional variants.
        $prime = locale_get_primary_language($row[0]);
        if (array_key_exists($prime, $langs)) {
          $langs[$prime]['visits'] += $row[1]; 
          $langs[$prime]['pageviews'] += $row[2] + $row[3]; 
        } else {
          $langs[$prime]['visits'] = $row[1]; 
          $langs[$prime]['pageviews'] = $row[2] + $row[3]; 
        } 
      }
			foreach ($langs as $prime => $counts) {
        echo '<tr>';
				echo '<td colspan="2">'.locale_get_display_name($prime, 'en').'</td><td colspan="2">'.$counts['visits'].'</td><td colspan="2">'.$counts['pageviews'].'</td>';
        echo '</tr>';
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
}

// Geo-location report.
function reportLocations() {
	global $client, $profileId, $period;

	// Max report length.
	$max = 10;

	// Date range
	$start = date("Y-m-d", strtotime("yesterday -$period days"));
	$end = date("Y-m-d", strtotime("yesterday"));

  echo '<tr>';
 	echo '<td colspan="6" style="background-color:PowderBlue">';
  echo '<span id="title">TOP '.$max.' COUNTRIES FOR LAST '.$period.' DAYS</span>';
  echo '</td>';
  echo '</tr>';
  echo '<tr>';
	echo '<td colspan="2">COUNTRY</td><td colspan="2">VISITS</td><td colspan="2">PAGEVIEWS</td>';
  echo '</tr>';

	$metrics = "ga:sessions,ga:pageviews,ga:screenviews";
	$dimensions = "ga:country";
	$optParams = array('max-results' => $max,
							 'sort' => '-ga:sessions',
							 'dimensions' => $dimensions);

	$service = new Google_Service_Analytics($client);
	try {
		$results = $service->data_ga->get($profileId,
                     $start,
                     $end, 
							$metrics,
							$optParams);
		if (!empty($results->rows)) {
			foreach ($results->rows as $row) {
        echo '<tr>';
        $row3 = $row[2]+$row[3];
				echo '<td colspan="2">'.$row[0].'</td><td colspan="2">'.$row[1].'</td><td colspan="2">'.$row3.'</td>';
        echo '</tr>';
			}
		}
	} catch(Exception $e) {
   	echo 'There was an error : - ' . $e->getMessage();
	}
}

function getEvents() {
 // Generate an event list for use with GA reports.
  $postData = array(
    'call'  =>'events',
    'token' =>''  //invalid token, so non-privileged token
  );
  // setup
  $ch = curl_init('https://pl.nlm.nih.gov/rest_endpoint');
  curl_setopt_array($ch, array(
      CURLOPT_POST => TRUE,
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_HTTPHEADER => array('Content-Type: application/json'),
      CURLOPT_POSTFIELDS => json_encode($postData),
      CURLOPT_SSL_VERIFYHOST => 0,
      CURLOPT_SSL_VERIFYPEER => 0
  ));
  // send
  $response = curl_exec($ch);
  // errors
  if($response === FALSE){
    die(curl_error($ch));
  }
  // decode
  $responseData = json_decode($response, TRUE);
  return $responseData;
}

function endsWith($string, $test)
{
    $strlen = strlen($string);
    $testlen = strlen($test);
    if ($testlen > $strlen) return false;
    return substr_compare($string, $test, $strlen - $testlen, $testlen) === 0;
}
?>
