<?
// LPF Landing Page
require_once('top.php');
// page
?>
<html>
<head>
  <? echo $gf; ?>
  <? echo $css ?>
  <? echo $ga; ?>
  <? echo $meta; ?>
  <? echo $favicon; ?>
  <title>Find Missing Persons During A Disaster | Lost Person Finder</title>
</head>
<body>
  <div class="content">
    <!--
    <div style="background-color: yellow; border: 2px solid red; margin: 4px; padding: 2px; font-weight: bold; text-align: center;">
    Due to the lapse in government funding, the information on this website may not be up to date, transactions submitted via the website may not be processed, and the agency may not be able to respond to inquiries until appropriations are enacted.
    </div>
    -->
    <div class="main">
      <div class="hero1">
        <h1>Lost Person Finder</h1>
      </div>
      <div class="hero2">
        <h2>Systems and products for family reunification</h2>
      </div>
    <div class="lpf-logo">
      </div>
      <div class="boxes">
      <a href="/PeopleLocator-ReUnite">
          <div class="pl">
            <div class="white">NLM PEOPLE LOCATOR<sup class="white">&reg;</sup> (pl.nlm.nih.gov)<br>
            and the ReUnite<sup class="white">&reg;</sup> App on Android and iOS</div>
          </div>
      </a>
      <a href="/TriageTrak-TriagePic">
          <div class="tt">
            <div class="white">TriageTrak (triagetrak.nlm.nih.gov) and the<br>
            TriagePic<sup class="white">&reg;</sup> App on Android, iOS, and Windows</div>
          </div>
      </a>
      </div>
      <div style="clear:both;"></div>
      <a href="http://nlm.nih.gov">
        <div class="nlm">National Library of Medicine</div>
      </a>
    </div>
  </div>
<?
// Footer 
require_once('bottom.php');
?>
</body>
</html>
