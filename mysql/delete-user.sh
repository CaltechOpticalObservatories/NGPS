#!/usr/bin/bash

# Script to delete a user (and their data) from NGPS mySQL
# mySQL login will be for user=gui, not the same as user being deleted

user=$1

# -z checks if var is empty
if [ -z "$user" ]; then
    echo "USAGE: ./delete-user.sh <USERNAME>"
    exit 1
fi

sqlout=/tmp/sqlout
sqlerr=/tmp/sqlerr

# Initial query to show user info
mysql -ugui -Dngps -p -t -e "set @username='${user}'; \
                  SELECT * FROM owner WHERE owner_id=@username; \
                  SELECT * FROM target_sets WHERE owner=@username;" >$sqlout 2>$sqlerr #>$sqlout

# -s checks if file is NOT empty
if [ -s $sqlerr ]; then
    cat $sqlerr
    echo "Problem logging in"
    exit 1
fi

# -z checks if var is empty
name_error=`grep $user $sqlout`
if [ -z "$name_error" ]; then
    echo "User not found: $user"
    exit 1
fi

cat $sqlout

echo ""
echo ""
read -p "Are you sure you want to DELETE user $1? (yes/no) " yn

case $yn in 
	yes ) echo Re-enter password to delete...;;
	* ) echo Exiting...;
		exit;;
esac

# Initial query to show user info
mysql -ugui -Dngps -p -t -e "set @username='${user}'; \
                  DELETE FROM owner WHERE owner_id=@username; \
                  DELETE FROM target_sets WHERE owner=@username; \
                  DELETE FROM targets WHERE owner=@username; 
                  SELECT owner_id FROM owner;"

