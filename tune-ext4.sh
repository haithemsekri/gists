#!/bin/bash

umount /media/hs/Build/
mkfs.ext4 -O dir_index,extent -i 8192  /dev/nvme1n1p10
e2label /dev/nvme1n1p10 Build
tune2fs -O ^has_journal  /dev/nvme1n1p10 
tune2fs -o journal_data_writeback  /dev/nvme1n1p10
e2fsck -f -y /dev/nvme1n1p10
dumpe2fs /dev/nvme1n1p10 | more
mount -o defaults,noatime,nodiratime,data=writeback,barrier=0,nobh,errors=remount-ro /dev/nvme1n1p10 /media/hs/Build/
# fio --name=basic --filename=/media/hs/Build/fiotestfile --size=1G --rw=randrw --bs=4k --direct=0
