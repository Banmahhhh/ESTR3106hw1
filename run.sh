#!/bin/bash
#
# help_run() To print help
#
help_run()
{
  echo "Usage: $0 -m -d"
  echo "Options: These are optional argument"
  echo " -m num of human player [0..2]"
  echo " -d demo values (if any of the above options are used "
  echo "                 their values are not taken)"
  exit 1
}
#
#Set default value for variable
man=1
isdef=0
#
# if [ $# -lt 1 ]; then
#   help_run
# fi
# while getopts m:d opt
# do
#   case "$opt" in
#     m) man="$OPTARG";;
#     d) isdef=1;;
#     \?) help_run;;
#   esac
# done
# if [ $isdef -eq 1 ]
# then
#   man=1
# fi
# #
# if [ $man -gt 2 ]
# then
#   echo "At most 2 human player"
#   man=2
# fi
#
# if ! pgrep -x "protocolRouter" > /dev/null
# then
# 	echo "Running protocolRouter ..."
# 	~/simpl/bin/protocolRouter &
# fi

# if ! pgrep -x "surrogateTcp" > /dev/null
# then
# 	echo "Running surrogateTcp ..."
# 	~/simpl/bin/surrogateTcp &
# fi

./game_admin &
sleep 1     #take some time for game admin to start
#
./display_admin &
sleep 1     #take some time for display admin to start
#
./courier 2 &

./input_admin &
sleep 1
./courier 0 &
./courier 1 &

./timer 1 &
./timer 2 &
./timer 3 &
# sleep 1
./painter &

./keyboard

# if [ $man -gt 0 ]
# then
#   ./keyboard
# fi
