<?


$postData = array(
  'call'  =>'events',
  'token' =>''
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
echo '<pre>'.print_r($responseData, true).'</pre>';
