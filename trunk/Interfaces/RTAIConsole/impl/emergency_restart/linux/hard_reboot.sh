#!/bin/bash
RTAI_LOC=/usr/realtime/modules
CONSOLE_LOC=/root/code/JET_Control/RTAIConsole/module

insmod $RTAI_LOC/rtai_hal.ko
insmod $RTAI_LOC/rtai_sched.ko
insmod $RTAI_LOC/rtai_sem.ko

cd $CONSOLE_LOC
./loadConsole.cmd
cd -

./emergency_restart.ex

