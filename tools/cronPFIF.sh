#!/bin/bash

exe_dir=/opt/pl/mod/pfif
log_dir=/tmp
#php_dir=/Applications/MAMP/bin/php/php7.1.12/bin

cd $exe_dir

$php_dir/php $exe_dir/cronexport.php        1>> $log_dir/export.out 2>> $log_dir/export.err
$php_dir/php $exe_dir/cronimport.php person 1>> $log_dir/import.out 2>> $log_dir/import.err
$php_dir/php $exe_dir/cronimport.php note   1>> $log_dir/import.out 2>> $log_dir/import.err
