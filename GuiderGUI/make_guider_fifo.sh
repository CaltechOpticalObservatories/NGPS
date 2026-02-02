#!/bin/bash
FIFO=/tmp/.guider_warning.fifo

# Check if script is already running
if pgrep -f "$(basename "$0")" | grep -vq $$; then
    echo $(basename "$0")" is already running, exiting."
    exit 0
fi

# Create the FIFO if it doesn't exist
if [ ! -p "$FIFO" ]; then
    if mkfifo -m 666 "$FIFO" 2>/dev/null; then
        :
    else
        mkfifo "$FIFO" || { echo "Failed to create FIFO"; exit 1; }
        chmod 666 "$FIFO" || { echo "Failed to chmod FIFO"; exit 1; }
    fi
fi


# Infinite loop listening for messages
while true; do
    if read -r MESSAGE < "$FIFO"; then
        # Escape double quotes
        ESCAPED_MESSAGE=$(printf '%s' "$MESSAGE" | sed 's/"/\\"/g')

        # Show Zenity warning on the current session
        if command -v zenity >/dev/null 2>&1; then
            zenity --warning --title="Guider" --text="$ESCAPED_MESSAGE" >/dev/null 2>&1 &
        fi
    fi
done
