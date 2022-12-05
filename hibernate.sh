#!/bin/bash

echo 8:7 > /sys/power/resume
ethtool -s enp6s0  wol g
echo 1 > /proc/sys/kernel/sysrq

sync
echo s > /proc/sysrq-trigger
sleep 0.1

echo 3 > /proc/sys/vm/drop_caches
sleep 0.1

echo u > /proc/sysrq-trigger

mount | grep -E '^/' | grep -v fuse | sed 's/ on / /g' |  sed 's/ type / -t /g'  |  sed 's/ (/ -o remount,/g'   |  sed 's/)//g' > /tmp/mount.backup

echo 0 > /sys/class/rtc/rtc0/wakealarm
rtcwake -m mem  -s 30
systemctl hibernate
cat /tmp/mount.backup | while read l; do  mount $l; done


#systemctl start systemd-hybrid-sleep.service
#systemctl start systemd-suspend-then-hibernate.service

