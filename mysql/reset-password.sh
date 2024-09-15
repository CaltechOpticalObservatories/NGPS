#!/usr/bin/bash

# Script to reset a user's password in NGPS GUI -- not to be confused with mysql users
# mySQL login will be for user=gui, not the same as user being deleted
# The new password will not be encrypted but will still work in the GUI.
# User should log in to the GUI using the new password then change it (then it will be encrypted)

user=$1
admin=gui
newpassword=RESET

# -z checks if var is empty
if [ -z "$user" ]; then
    echo "USAGE: ./reset-password.sh <USERNAME>"
    exit 1
fi

sqlout=/tmp/sqlout
sqlerr=/tmp/sqlerr

# Initial query to show user info
echo "Enter mysql password for $admin"
mysql -u$admin -Dngps -p -t -e "set @username='${user}'; \
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
read -p "Are you sure you want to RESET PASSWORD for user $user? (yes/no) " yn

case $yn in 
	yes ) echo Re-enter mysql $admin password to continue...;;
	* ) echo Exiting...;
		exit;;
esac

echo ""

# Do it
mysql -u$admin -Dngps -p -t -e "UPDATE owner SET password = '${newpassword}' WHERE owner_id = '${user}'; \
							SELECT * FROM owner WHERE owner_id = '${user}';" 
