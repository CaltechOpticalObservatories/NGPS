#!/usr/bin/bash

# Script to list users in NGPS GUI
# mySQL login will be for user=gui

mysql -ugui -Dngps -p -t -e "SELECT owner_id FROM owner;"

