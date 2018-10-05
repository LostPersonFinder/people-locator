DROP PROCEDURE IF EXISTS `PLSearch`;
delimiter //
CREATE DEFINER=`root`@`localhost` PROCEDURE `PLsearch` (
  IN `searchTerms` TEXT,
  IN `statusFilter` VARCHAR(100),
  IN `genderFilter` VARCHAR(100),
  IN `ageFilter` VARCHAR(100),
  IN `incidentName` VARCHAR(100),
  IN `sortBy` VARCHAR(100),
  IN `pageStart` INT,
  IN `perPage` INT,
  IN `hasImage` BOOLEAN,
  IN `since` VARCHAR(48),
  IN `animal` INT,
  OUT `allCount` INT
)
BEGIN
  DECLARE image_join_prefix varchar(10) DEFAULT 'LEFT ';
  DECLARE animal_clause varchar(20) DEFAULT '';
  DECLARE sortByKeyword varchar(10) DEFAULT 'ORDER BY ';
  DECLARE sortByValue varchar(80) DEFAULT `sortBy`;
  DECLARE date_clause varchar(48) DEFAULT '';
  IF animal = 0 THEN
    SET animal_clause = 'tn.animal IS NULL AND';
  ELSEIF animal = 1 THEN
    SET animal_clause = 'tn.animal IS NOT NULL AND';
  END IF;
  IF hasImage THEN
    SET image_join_prefix = '';
  END IF;
  IF sortBy LIKE 'similarity%' THEN
    SET sortByKeyword = '';
    SET sortByValue = '';
  END IF;
  IF since != '' THEN
    SET date_clause = CONCAT('AND ps.last_updated > "',since,'"');
  END IF;
  DROP TABLE IF EXISTS tmp_names;
  IF searchTerms = '' THEN
    CREATE TEMPORARY TABLE tmp_names AS (
      SELECT SQL_NO_CACHE pu.*
      FROM person_uuid pu
      JOIN incident i  ON (pu.incident_id = i.incident_id AND i.shortname = incidentName)
      LIMIT 1000000
    );
  ELSEIF searchTerms = 'unknown' THEN
    CREATE TEMPORARY TABLE  tmp_names AS (
      SELECT SQL_NO_CACHE pu.*
      FROM person_uuid pu
      JOIN incident i  ON (pu.incident_id = i.incident_id AND i.shortname = incidentName)
      WHERE (full_name = '' OR full_name IS NULL)
      LIMIT 1000000
    );
  ELSEIF locate('/', searchTerms) = 0 THEN
    CREATE TEMPORARY TABLE  tmp_names AS (
      SELECT SQL_NO_CACHE pu.*
      FROM person_uuid pu
      JOIN incident i  ON (pu.incident_id = i.incident_id AND i.shortname = incidentName)
      WHERE full_name like CONCAT(REPLACE(searchTerms,'*','%'),'%')
      OR family_name like CONCAT(REPLACE(searchTerms,'*','%'),'%')
      LIMIT 1000000
    );
  ELSE
    CREATE TEMPORARY TABLE  tmp_names AS (
      SELECT SQL_NO_CACHE pu.*
      FROM person_uuid pu
      JOIN incident i  ON (pu.incident_id = i.incident_id AND i.shortname = incidentName)
      WHERE FIND_IN_SET(pu.p_uuid, searchTerms)
      ORDER BY FIND_IN_SET(pu.p_uuid, searchTerms)
      LIMIT 1000000
    );
  END IF;
  SET @sqlString = CONCAT("
    SELECT 
      SQL_CALC_FOUND_ROWS 
      `tn`.`p_uuid`,
      `tn`.`full_name`,
      `tn`.`given_name`,
      `tn`.`family_name`,
      (CASE WHEN `ps`.`opt_status` NOT IN ('ali', 'mis', 'inj', 'dec', 'fnd') THEN 'unk' ELSE `ps`.`opt_status` END) AS `opt_status`,
      CONVERT_TZ(ps.last_updated,'America/New_York','UTC') AS updated,
      (CASE WHEN `pd`.`opt_gender` NOT IN ('mal', 'fml', 'cpx') OR `pd`.`opt_gender` IS NULL THEN 'unk' ELSE `pd`.`opt_gender` END) AS `opt_gender`,
      `pd`.`years_old`,
      `pd`.`minAge`,
      `pd`.`maxAge`,
      `i`.`image_height`,
      `i`.`image_width`,
      `i`.`url`,
      `i`.`url_thumb`,
      `i`.`color_channels`,
      `pd`.`last_seen`,
      `pd`.`other_comments` AS `comments`
    FROM tmp_names tn
    JOIN person_status ps
      ON (tn.p_uuid = ps.p_uuid ", date_clause, " AND INSTR(?,
        (CASE WHEN ps.opt_status NOT IN ('ali', 'mis', 'inj', 'dec', 'fnd') OR ps.opt_status IS NULL THEN 'unk' ELSE ps.opt_status END)
      ))
    JOIN person_details pd 
      ON (tn.p_uuid = pd.p_uuid AND INSTR(?,
        (CASE WHEN `opt_gender` NOT IN ('mal', 'fml', 'cpx') OR `opt_gender` IS NULL THEN 'unk' ELSE `opt_gender` END)
      )
    AND INSTR(?,
      (CASE WHEN CONVERT(`pd`.`years_old`, UNSIGNED INTEGER) IS NOT NULL THEN
        (CASE WHEN `pd`.`years_old` < 18 THEN 'youth' WHEN `pd`.`years_old` >= 18 THEN 'adult' END)
        WHEN CONVERT(`pd`.`minAge`, UNSIGNED INTEGER) IS NOT NULL
        AND CONVERT(`pd`.`maxAge`, UNSIGNED INTEGER) IS NOT NULL
        AND `pd`.`minAge` < 18
        AND `pd`.`maxAge` >= 18
        THEN 'both'
        WHEN CONVERT(`pd`.`minAge`, UNSIGNED INTEGER) IS NOT NULL
        AND `pd`.`minAge` >= 18
        THEN 'adult'
        WHEN CONVERT(`pd`.`maxAge`, UNSIGNED INTEGER) IS NOT NULL
        AND `pd`.`maxAge` < 18
        THEN 'youth'
        ELSE 'unknown'
        END)
      )
    )
    ", image_join_prefix, "JOIN image i ON (tn.p_uuid = i.p_uuid AND i.principal = TRUE)
    WHERE ",animal_clause," (tn.expiry_date > NOW() OR tn.expiry_date IS NULL)
    ",sortByKeyword,sortByValue,"
    LIMIT ?, ?;"
  );
  PREPARE stmt FROM @sqlString;
  SET @statusFilter = statusFilter;
  SET @genderFilter = genderFilter;
  SET @ageFilter = ageFilter;
  SET @pageStart = pageStart;
  SET @perPage = perPage;
  SET NAMES utf8;
  SET SESSION binlog_format = ROW;  
  EXECUTE stmt USING @statusFilter, @genderFilter, @ageFilter, @pageStart, @perPage;
  SELECT FOUND_ROWS() INTO allCount;
  DEALLOCATE PREPARE stmt;
  DROP TABLE tmp_names;
END //
delimiter ;
