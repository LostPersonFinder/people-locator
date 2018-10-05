DROP VIEW `image_search`;
CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `image_search` AS select
  `i`.`image_id` AS `image_id`,
  `i`.`url` AS `url`,
  `i`.`url_thumb` AS `url_thumb`,
  `i`.`image_height` AS `image_height`,
  `i`.`image_width` AS `image_width`,
  `i`.`color_channels` AS `color_channels`,
  `i`.`original_filename` AS `original_filename`,
  `i`.`created` AS `created`,
  `is`.`gt_initial_person` AS `gt_initial_person`,
  `is`.`gt_final_person` AS `gt_final_person`,
  `is`.`initial_regions` AS `initial_regions`,
  `is`.`final_regions` AS `final_regions`,
  `is`.`initial_updated_time` AS `initial_updated_time`,
  `is`.`final_updated_time` AS `final_updated_time`,
  `is`.`groundTruthStatus` AS `groundTruthStatus`,
  `is`.`record_updated` AS `record_updated`,
  `pu`.`p_uuid` AS `p_uuid`,
  `pu`.`full_name` AS `full_name`,
  `pu`.`given_name` AS `given_name`,
  `pu`.`family_name` AS `family_name`,
  `pu`.`alternate_names` AS `alternate_names`,
  `pu`.`expiry_date` AS `expiry_date`,
  `ps`.`last_updated` AS `updated`,
  `ps`.`last_updated_db` AS `updated_db`,
  (case when (`ps`.`opt_status` not in ('ali','mis','inj','dec','unk','fnd')) then 'unk' else `ps`.`opt_status` end) AS `opt_status`,
  (case when ((`pd`.`opt_gender` not in ('mal','fml','cpx')) or isnull(`pd`.`opt_gender`)) then 'unk' else `pd`.`opt_gender` end) AS `opt_gender`,
  (case when isnull(cast(`pd`.`years_old` as unsigned)) then -(1) else `pd`.`years_old` end) AS `years_old`,
  (case when isnull(cast(`pd`.`minAge` as unsigned)) then -(1) else `pd`.`minAge` end) AS `minAge`,
  (case when isnull(cast(`pd`.`maxAge` as unsigned)) then -(1) else `pd`.`maxAge` end) AS `maxAge`,
  (case when (cast(`pd`.`years_old` as unsigned) is not null) 
    then (case when (`pd`.`years_old` < 18) 
    then 'youth' when (`pd`.`years_old` >= 18) then 'adult' end)
    when ((cast(`pd`.`minAge` as unsigned) is not null) and (cast(`pd`.`maxAge` as unsigned) is not null) and (`pd`.`minAge` < 18) and (`pd`.`maxAge` >= 18)) then 'both' 
    when ((cast(`pd`.`minAge` as unsigned) is not null) and (`pd`.`minAge` >= 18)) then 'adult' 
    when ((cast(`pd`.`maxAge` as unsigned) is not null) and (`pd`.`maxAge` < 18)) then 'youth' else 'unknown' end) AS `ageGroup`,
  `inc`.`shortname` AS `shortname`,
  `inc`.`name` AS `name`,
  `pd`.`other_comments` AS `comments`,
  `pd`.`last_seen` AS `last_seen`
from 
(
  (
    (
      (
        (`image` `i` join `person_uuid` `pu` on((`pu`.`p_uuid` = `i`.`p_uuid`)))
        join `person_status` `ps` on((`ps`.`p_uuid` = `i`.`p_uuid`))
      ) 
      join `person_details` `pd` on((`pd`.`p_uuid` = `i`.`p_uuid`))
    ) 
    join `incident` `inc` on((`inc`.`incident_id` = `pu`.`incident_id`))
  ) 
  left join `imagestats` `is` on((`is`.`image_id` = `i`.`image_id`))
);
