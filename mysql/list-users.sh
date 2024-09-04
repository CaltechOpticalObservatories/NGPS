#!/usr/bin/bash

# Script to list users in NGPS GUI -- not to be confused with mysql users
# mySQL login will be for user=gui

mysql -ugui -Dngps -p -t -e "SELECT owner_id FROM owner;"

