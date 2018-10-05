<?
/**
 * @name      test module
 * @author    pl@miernicki.com
 * @about     Developed by the U.S. National Library of Medicine
 * @link      https://gitlab.com/tehk/people-locator
 * @license   https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

// main controller
function taupo_test_default() {
?>
<div style="background-color: white; padding: 4em;">
  <ul>
    <li><a href="?mod=test&act=gmap">paper-input google map autocomplete</a><br></li>
    <li><a href="?mod=test&act=ac">paper-input-autocomplete</a><br></li>
  </ul>
</div>
<?
}
// google maps autocomple test
function taupo_test_gmap() {
?>
<link rel="import" href="assets/bower_components/google-map/google-map.html">
<link rel="import" href="assets/bower_components/google-map/google-map-search.html">
<div style="background-color: white; padding: 4em;">
  <paper-input id="search"></paper-input>
  <google-map style="width: 100%; height: 600px;" latitude="37.779" longitude="-122.3892" min-zoom="9" max-zoom="11" language="en"></google-map>
</div>
<script>
  var autocomplete;
  function init() {
    autocomplete = new google.maps.places.Autocomplete((document.getElementById('search')), {types:['geocode']});
    google.maps.event.addListener(autocomplete, 'place_changed', callBack);
  }
  function callBack() {
    var place = autocomplete.getPlace();
    $('google-map').prop('latitude',  place.geometry.location.lat());
    $('google-map').prop('longitude', place.geometry.location.lng());
    console.log(place.geometry.location.lat());
    console.log(place.geometry.location.lng());
  }
  $(document).ready(function() {
    setTimeout(function(){ init(); }, 1000);
  });
</script>
<?
}
// paper-input-autocomplete test
function taupo_test_ac() {
?>
<link rel="import" href="assets/bower_components/paper-input-autocomplete/paper-input-autocomplete.html">
<div style="background-color: white; padding: 4em;">
  <paper-input-autocomplete id="fruits" label="Fruits" suggestions-in-overlay="true"></paper-input-autocomplete>
  <button onclick="whichFruit()">Which fruit?</button><br/>
  (data: Apple Orange Bananas)
  <br/><br/><br/>
  <paper-input-autocomplete id="vegetables" label="Vegetables" search-property="name"></paper-input-autocomplete>
  <button onclick="whichVegetable()">Which vegetable?</button><br>
  (data: Cabbage, Carrot, Potato)
</div>
<script>
  document.getElementById('fruits').source = ['Apple', 'Orange', 'Bananas'];
  document.getElementById('vegetables').source = [
    {name: 'Cabbage', scientificName: 'Brassica oleracea'},
    {name: 'Carrot', scientificName: 'Daucus carota'},
    {name: 'Potato', scientificName: 'Solanum tuberosum'},
  ];
  function whichFruit() {
    alert(document.getElementById('fruits').getValue());
  }
  function whichVegetable() {
    alert(JSON.stringify(document.getElementById('vegetables').getValue()));
  }
</script>
<?
}
