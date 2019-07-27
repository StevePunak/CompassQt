# CompassQt
This is a QT-based daemon meant to run on a Raspberry PI which provides compass support via MQTT

## This is a Qt based daemon process to handle IC input and propagate to MQTT

The daemon publishes a single double (little-endian) at the given interval

The systemd service unit and the default config file are in docs/
The service unit should be placed at /lib/systemd/system, and the config file at /etc/compass/compass.conf

