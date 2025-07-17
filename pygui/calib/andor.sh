#!/bin/bash

# Move to the working directory
cd /home/observer/focus || exit 1

# Get the argument or default to "focus*"
pattern="${1:-focus*}"

# Find the most recent matching FITS file
latest_file=$(ls -1t /data/latest/acam/${pattern}.fits 2>/dev/null | head -n 1)

# Check if a matching file was found
if [ -z "$latest_file" ]; then
    echo "No matching FITS files found for pattern: ${pattern}"
    exit 1
fi

# Run the focus script on the latest file
/home/developer/Software/run/focus_andor.py -fk TELFOCUS "$latest_file"
