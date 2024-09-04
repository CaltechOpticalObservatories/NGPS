#!/bin/bash

# Backup mysql data before making or pushing changes
# Usage:  ./backup-all.sh
# You will need the mysql ROOT password

tag=`date -Iminutes`
tag=${tag::-6}							# remove UTC conversion "-07:00"
savedir=/home/developer/Software/mysql  # schema backups to track in git
skeedir=$savedir/local-schema 			# skeema-style backup for pushing changes
backupdir=/tmp  						# full backups to protect against mistakes
deployhost=ngps.caltech.edu


read -sp 'ROOT password for mysql: ' password

# 1. FULL BACKUP (do before making or pushing ANY changes to a db with important DATA)

echo ""
echo "BACKING UP ALL DATABASES TO $backupdir ..."
#deployed
mysqldump -u root -p$password -h $deployhost --all-databases > $backupdir/mysql_deploy_fullbackup_$tag.sql
mysqldump -u root -p$password -h $deployhost mysql user > $backupdir/mysql_deploy_allusers_$tag.sql

#local
mysqldump -u root -p$password --all-databases > $backupdir/mysql_local_fullbackup_$tag.sql
mysqldump -u root -p$password mysql user > $backupdir/mysql_local_allusers_$tag.sql


# 2. SCHEMA and USER BACKUP (use to log changes to dev version)

echo ""
echo "BACKING UP LOCAL SCHEMA..."

mysqlpump -u root -p$password --exclude-databases=% --add-drop-user --users > $savedir/pump-all-users_privileges.sql  
# dump and pump are different commands, not a typo

# Use --no-data option to backup schema only (table structures, no entries):

mysqldump -u root -p$password --no-data --all-databases > $savedir/mysql-allschema.sql

cd $skeedir
skeema pull local -p$password
cd $savedir

echo ""
echo " ~~ COMMIT CHANGES TO GIT BEFORE PUSHING TO DEPLOY MACHINE"
echo ""