#!/bin/sh
#
# CircleMUD 3.0 autorun script
# Contributions by Fred Merkel, Stuart Lamble, and Jeremy Elson
# Copyright (c) 1996 The Trustees of The Johns Hopkins University
# All Rights Reserved
# See license.doc for more information
#
#############################################################################
#
# This script can be used to run CircleMUD over and over again (i.e., have it
# automatically reboot if it crashes).  It will run the game, and copy some
# of the more useful information from the system logs to the 'log' directory
# for safe keeping.
#
# You can control the operation of this script by creating and deleting files
# in Circle's root directory, either manually or by using the 'shutdown'
# command from within the MUD.
#
# Creating a file called .fastboot makes the script wait only 5 seconds
# between reboot attempts instead of the usual 60.  If you want a quick
# reboot, use the "shutdown reboot" command from within the MUD.
#
# Creating a file called .killscript makes the script terminate (i.e., stop
# rebooting the MUD).  If you want to shut down the MUD and make it stay
# shut down, use the "shutdown die" command from within the MUD.
#
# Finally, if a file called pause exists, the script will not reboot the MUD
# again until pause is removed.  This is useful if you want to turn the MUD
# off for a couple of minutes and then bring it back up without killing the
# script.  Type "shutdown pause" from within the MUD to activate this feature.
#
# Resolve wdII_dir relative to this script's location
wdII_dir="$(cd "$(dirname "$0")" && pwd)"
# The port on which to run the MUD
PORT=2022
# Default flags to pass to the MUD server (see admin.txt for a description
# of all flags).
FLAGS='-q'
#############################################################################
while ( : )
do

  DATE=`date`
  echo "autorun starting game $DATE" >> $wdII_dir/syslog
  echo "running bin/circle $FLAGS $PORT" >> $wdII_dir/syslog

  $wdII_dir/bin/circle $FLAGS $PORT >> $wdII_dir/syslog

  tail -30 $wdII_dir/syslog > $wdII_dir/syslog.CRASH

  fgrep "self-delete" $wdII_dir/syslog >> $wdII_dir/log/delete
  fgrep "death trap" $wdII_dir/syslog >> $wdII_dir/log/dts
  fgrep "killed" $wdII_dir/syslog >> $wdII_dir/log/rip
  fgrep "Running" $wdII_dir/syslog >> $wdII_dir/log/restarts
  fgrep "advanced" $wdII_dir/syslog >> $wdII_dir/log/levels
  fgrep "equipment lost" $wdII_dir/syslog >> $wdII_dir/log/rentgone
  fgrep "usage" $wdII_dir/syslog >> $wdII_dir/log/usage
  fgrep "new player" $wdII_dir/syslog >> $wdII_dir/log/newplayers
  fgrep "SYSERR" $wdII_dir/syslog >> $wdII_dir/log/errors
  fgrep "(GC)" $wdII_dir/syslog >> $wdII_dir/log/godcmds
  fgrep "Bad PW" $wdII_dir/syslog >> $wdII_dir/log/badpws

  rm $wdII_dir/log/syslog.1
  mv $wdII_dir/log/syslog.2 $wdII_dir/log/syslog.1
  mv $wdII_dir/log/syslog.3 $wdII_dir/log/syslog.2
  mv $wdII_dir/log/syslog.4 $wdII_dir/log/syslog.3
  mv $wdII_dir/log/syslog.5 $wdII_dir/log/syslog.4
  mv $wdII_dir/log/syslog.6 $wdII_dir/log/syslog.5
  mv $wdII_dir/syslog       $wdII_dir/log/syslog.6
  touch $wdII_dir/syslog

  if [ -r .killscript ]; then
    DATE=`date`;
    echo "autoscript killed $DATE"  >> $wdII_dir/syslog
    rm .killscript
    exit
  fi

  if [ ! -r .fastboot ]; then
    sleep 60
  else
    rm .fastboot
    sleep 5
  fi

  while [ -r pause ]
  do
    sleep 60
  done
done
