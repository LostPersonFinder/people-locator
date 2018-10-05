<?
/**
 * @name     Person Finder Interchange Format
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$pfif_conf['services'] = array();

// TODO: These methods are Google-specific. They should be generalized as needed.
function add_pfif_service($repository) {
   global $pfif_conf, $conf;

   $service_name = $repository->name;
   // Stash repository here for getting at harvest log, etc.
   $pfif_conf['services'][$service_name]['repository'] = $repository;
   // The NLM PF uses this parameter to specify the disaster.
   $pfif_conf['services'][$service_name]['subdomain'] = $repository->subdomain;
   // TODO: Move to repository table.
   $pfif_conf['services'][$service_name]['version'] = '1.4';
   if (empty($repository->subdomain)) {
   	$pfif_conf['services'][$service_name]['auth_key'] = $conf['gpf_api_key'];
   } else {
   	$pfif_conf['services'][$service_name]['auth_key'] = $conf['nlm_api_key'];
   }
   $pfif_conf['services'][$service_name]['incident_id'] = $repository->incident_id;
   if ($repository->role == 'source') {
       $pfif_conf['services'][$service_name]['feed_url'] = $repository->base_url.'/feeds/'.$repository->resource_type;
       $pfif_conf['services'][$service_name]['read_url'] = $repository->base_url.'/api/read';
       $pfif_conf['services'][$service_name]['max_results'] = '200';
   } else if ($repository->role == 'sink') {
       // TODO: Change this to req_url.
       $pfif_conf['services'][$service_name]['post_url'] = $repository->base_url.'/api/write';
   } else {
       // ERROR: invalid role
   }
}
