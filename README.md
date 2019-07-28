# CompassQt
This is a QT-based daemon meant to run on a Raspberry PI which provides compass support via MQTT

## This is a Qt based daemon process to handle IC input and propagate to MQTT

The daemon publishes a single double (little-endian) at the given interval

The systemd service unit and the default config file are in docs/
The service unit should be placed at /lib/systemd/system, and the config file at /etc/compass/compass.conf

# Dependencies
Was built targeting QT 5.13 in a cross-compile configuration, although other versions should work.
It depends on the KanoopCommon library at git@github.com:StevePunak/KanoopCommonQt.git for logging
This can be build without external dependecies other than QT

# Note
The LSM9DS1 interface is the work of Jim Lindblom @ SparkFun Electronics
https://github.com/sparkfun/LSM9DS1_Breakout

It is included in source form here so as not to require the user to download and build another library
