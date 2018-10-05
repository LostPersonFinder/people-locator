DROP PROCEDURE IF EXISTS `delete_reported_person`;
delimiter //
CREATE DEFINER=`root`@`localhost` PROCEDURE `delete_reported_person`(IN `id` VARCHAR(128), IN `deleteNotes` BOOLEAN)
BEGIN
  DELETE person_uuid.* FROM person_uuid WHERE p_uuid = id;
  DELETE pfif_person.* FROM pfif_person WHERE p_uuid = id;
  IF deleteNotes THEN
   DELETE pfif_note.* FROM pfif_note WHERE p_uuid = id;
   UPDATE pfif_note SET linked_person_record_id = NULL WHERE linked_person_record_id = id;
  END IF;
END //
delimiter ;
