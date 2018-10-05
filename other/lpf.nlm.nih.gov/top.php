<?
// common LPF page top

// load and compress css
$buffer = file_get_contents('styles.css');
$buffer = preg_replace('!/\*[^*]*\*+([^/][^*]*\*+)*/!', '', $buffer);                      // Remove comments
$buffer = str_replace(': ', ':', $buffer);                                                 // Remove space after colons
$buffer = str_replace(array("\r\n", "\r", "\n", "\t", '  ', '    ', '    '), '', $buffer); // Remove whitespace
$css    = "<style>".$buffer."</style>\n";
// headers
ob_start("ob_gzhandler"); 	// Enable gzip compression
header("Content-Type: text/html; charset=utf-8");
header("Cache-Control: no-store, must-revalidate, max-age=0");
header("Expires: ".date('D, j M Y H:i:s')." GMT", time()+(24*60*60*32)); // expire in 32 days "Expires: Sat, 26 Jul 1997 05:00:00 GMT"
// ga
$ga="<script>(function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)})(window,document,'script','//www.google-analytics.com/analytics.js','ga');ga('create', 'UA-49715038-1', 'nih.gov');ga('send', 'pageview')</script>";
// gf
$gf='<link href="//fonts.googleapis.com/css?family=Open+Sans:300italic,300,400italic,400,600italic,600,700italic,700,800italic,800" rel="stylesheet" type="text/css">';
// meta tags
$meta = '<meta name="apple-itunes-app" content="app-id=368052994">';
$meta .= '<meta name="description" content="Find lost persons during a disaster using our missing persons registry. App users can download our ReUnite app (iOS and Android).">';
// We use realfavicongenerator.net to generate our favicon headers.
$favicon = '
  <link rel="apple-touch-icon" sizes="57x57" href="assets/icons/apple-touch-icon-57x57.png">
  <link rel="apple-touch-icon" sizes="60x60" href="assets/icons/apple-touch-icon-60x60.png">
  <link rel="apple-touch-icon" sizes="72x72" href="assets/icons/apple-touch-icon-72x72.png">
  <link rel="apple-touch-icon" sizes="76x76" href="assets/icons/apple-touch-icon-76x76.png">
  <link rel="apple-touch-icon" sizes="114x114" href="assets/icons/apple-touch-icon-114x114.png">
  <link rel="apple-touch-icon" sizes="120x120" href="assets/icons/apple-touch-icon-120x120.png">
  <link rel="apple-touch-icon" sizes="144x144" href="assets/icons/apple-touch-icon-144x144.png">
  <link rel="apple-touch-icon" sizes="152x152" href="assets/icons/apple-touch-icon-152x152.png">
  <link rel="apple-touch-icon" sizes="180x180" href="assets/icons/apple-touch-icon-180x180.png">
  <link rel="icon" type="image/png" href="assets/icons/favicon-32x32.png" sizes="32x32">
  <link rel="icon" type="image/png" href="assets/icons/favicon-194x194.png" sizes="194x194">
  <link rel="icon" type="image/png" href="assets/icons/favicon-96x96.png" sizes="96x96">
  <link rel="icon" type="image/png" href="assets/icons/android-chrome-192x192.png" sizes="192x192">
  <link rel="icon" type="image/png" href="assets/icons/favicon-16x16.png" sizes="16x16">
  <link rel="manifest" href="assets/icons/manifest.json">
  <link rel="mask-icon" href="assets/icons/safari-pinned-tab.svg" color="#5bbad5">
  <meta name="msapplication-TileColor" content="#da532c">
  <meta name="msapplication-TileImage" content="assets/icons/mstile-144x144.png">
  <meta name="theme-color" content="#ffffff">
  ';
