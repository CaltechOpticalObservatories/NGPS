#!/usr/bin/bash

# Script to add MySQL users for NGPS
# Based on user definitions from pump-all-users_privileges.sql

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <username> [password]"
    echo ""
    echo "Common NGPS users:"
    echo "  gui       - GUI application user (full access to targets, target_sets, owner)"
    echo "  obseq     - Observation sequencer user (read/update targets)"
    echo "  grafana   - Grafana monitoring user (read-only telemetry)"
    echo "  telemlogger - Telemetry logging user (insert telemetry data)"
    exit 1
fi

USERNAME=$1
PASSWORD=${2:-}

# If no password provided, prompt for it
if [ -z "$PASSWORD" ]; then
    echo "Enter password for user '$USERNAME':"
    read -s PASSWORD
    echo ""
fi

echo "Enter MySQL root password:"
read -s ROOTPASS
echo ""

case $USERNAME in
    gui)
        echo "Creating 'gui' user with full access to ngps database..."
        mysql -u root -p"$ROOTPASS" <<EOF
CREATE USER IF NOT EXISTS 'gui'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT USAGE ON *.* TO \`gui\`@\`localhost\`;
GRANT SELECT ON \`ngps\`.\`completed_obs\` TO \`gui\`@\`localhost\`;
GRANT SELECT ON \`ngps\`.\`completed_observations\` TO \`gui\`@\`localhost\`;
GRANT ALL PRIVILEGES ON \`ngps\`.\`owner\` TO \`gui\`@\`localhost\`;
GRANT ALL PRIVILEGES ON \`ngps\`.\`target_sets\` TO \`gui\`@\`localhost\`;
GRANT ALL PRIVILEGES ON \`ngps\`.\`targets_dev\` TO \`gui\`@\`localhost\`;
GRANT ALL PRIVILEGES ON \`ngps\`.\`targets\` TO \`gui\`@\`localhost\`;
FLUSH PRIVILEGES;
EOF
        ;;

    obseq)
        echo "Creating 'obseq' user (observation sequencer)..."
        mysql -u root -p"$ROOTPASS" <<EOF
CREATE USER IF NOT EXISTS 'obseq'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT USAGE ON *.* TO \`obseq\`@\`localhost\`;
GRANT INSERT ON \`ngps\`.\`completed_obs\` TO \`obseq\`@\`localhost\`;
GRANT INSERT ON \`ngps\`.\`completed_observations\` TO \`obseq\`@\`localhost\`;
GRANT SELECT ON \`ngps\`.\`target_sets\` TO \`obseq\`@\`localhost\`;
GRANT SELECT, UPDATE ON \`ngps\`.\`targets\` TO \`obseq\`@\`localhost\`;
FLUSH PRIVILEGES;
EOF
        ;;

    grafana)
        echo "Creating 'grafana' user (read-only telemetry)..."
        mysql -u root -p"$ROOTPASS" <<EOF
CREATE USER IF NOT EXISTS 'grafana'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT USAGE ON *.* TO \`grafana\`@\`localhost\`;
GRANT SELECT ON \`telemetry\`.* TO \`grafana\`@\`localhost\`;
FLUSH PRIVILEGES;
EOF
        ;;

    telemlogger)
        echo "Creating 'telemlogger' user (telemetry logging)..."
        mysql -u root -p"$ROOTPASS" <<EOF
CREATE USER IF NOT EXISTS 'telemlogger'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT USAGE ON *.* TO \`telemlogger\`@\`localhost\`;
GRANT INSERT, CREATE ON \`telemetry\`.* TO \`telemlogger\`@\`localhost\`;
FLUSH PRIVILEGES;
EOF
        ;;

    *)
        echo "Unknown user type: $USERNAME"
        echo "Creating generic user with no privileges (you'll need to grant manually)..."
        mysql -u root -p"$ROOTPASS" <<EOF
CREATE USER IF NOT EXISTS '$USERNAME'@'localhost' IDENTIFIED BY '$PASSWORD';
GRANT USAGE ON *.* TO \`$USERNAME\`@\`localhost\`;
FLUSH PRIVILEGES;
EOF
        echo "User created. Grant privileges manually with:"
        echo "  GRANT <privileges> ON <database>.<table> TO \`$USERNAME\`@\`localhost\`;"
        ;;
esac

if [ $? -eq 0 ]; then
    echo "User '$USERNAME' created successfully!"
else
    echo "Error creating user '$USERNAME'"
    exit 1
fi
