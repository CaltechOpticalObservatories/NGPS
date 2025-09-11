#!/bin/bash
FIFO=/tmp/.slicev_warning.fifo

# Check if script is already running
if pgrep -f "$(basename "$0")" | grep -vq $$; then
    echo $(basename "$0")" is already running, exiting."
    exit 0
fi

# Create the FIFO if it doesn't exist
if [ ! -p "$FIFO" ]; then
    mkfifo --mode=a=rw "$FIFO" || { echo "Failed to create FIFO"; exit 1; }
fi


# Infinite loop listening for messages
while true; do
    if read -r MESSAGE < "$FIFO"; then
        # Escape double quotes
        ESCAPED_MESSAGE=$(printf '%s' "$MESSAGE" | sed 's/"/\\"/g')

        # Show Zenity warning on the current session
        /usr/bin/zenity --warning --title="SliceCam" --text="$ESCAPED_MESSAGE" >/dev/null 2>&1 &
    fi
done

