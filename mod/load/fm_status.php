<?
// This script serves as the Site24x7 health monitor.
// It should report "SYSTEM IS UP" if healthy.

if (!taupo_facematch_status()) {
	echo "SYSTEM IS DOWN!";
	exit;
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Uptime</title>
</head>
<body>
SYSTEM IS UP
</body>
</html>
