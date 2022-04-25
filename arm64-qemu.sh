sudo apt-get install --no-install-recommends \
	git-email libaio-dev libbluetooth-dev libbrlapi-dev libbz2-dev \
	libcap-dev libcap-ng-dev libcurl4-gnutls-dev libgtk-3-dev \
	libibverbs-dev libjpeg8-dev libncurses5-dev libnuma-dev \
	librbd-dev librdmacm-dev \
	libsasl2-dev libsdl1.2-dev libseccomp-dev libsnappy-dev libssh2-1-dev \
	libvde-dev libvdeplug-dev libvte-2.91-dev libxen-dev liblzo2-dev \
	valgrind xfslibs-dev libnfs-dev libiscsi-dev screen ninja-build bison flex
	

wget http://sources.buildroot.net/qemu/qemu-3.1.0.tar.xz
tar -xvf qemu-3.1.0.tar.xz
cd qemu-3.1.0
./configure --target-list=aarch64-softmmu
make -j8

export PATH=$PATH:/home/hs/qemu/qemu-3.1.0/aarch64-softmmu/
qemu-system-aarch64 --version

wget http://sources.buildroot.net/qemu/qemu-6.2.0.tar.xz
tar -xvf qemu-6.2.0.tar.xz
cd qemu-6.2.0
./configure --target-list=aarch64-softmmu
make -j8

export PATH=$PATH:/home/hs/qemu/qemu-6.2.0/build/aarch64-softmmu
qemu-system-aarch64 --version

wget https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu.tar.xz
tar -xf gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu.tar.xz
mv gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu gcc-aarch64-linux-11.2
export PATH=$PATH:/home/hs/qemu-work/gcc-aarch64-linux-11.2/bin
aarch64-none-linux-gnu-gcc --version

--------------------------------------------------------------------------------------
wget https://www.busybox.net/downloads/busybox-1.34.1.tar.bz2
tar -xf busybox-1.34.1.tar.bz2
cd busybox-1.34.1

make O=/home/hs/qemu-work/busybox-1.34.1-build/ ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
make O=/home/hs/qemu-work/busybox-1.34.1-build/ ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
make O=/home/hs/qemu-work/busybox-1.34.1-build/ ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- -j8

make O=/home/hs/qemu-work/busybox-1.34.1-build/ ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- install

cd /home/hs/qemu-work/busybox-1.34.1-build/_install
mkdir -p proc sys dev  etc/init.d

cat etc/init.d/rcS
#! /bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
/sbin/mdev -s

chmod +x etc/init.d/rcS

find . | cpio -o --format=newc > ../../rootfs.img
cd ../..
gzip -c rootfs.img > rootfs.img.gz


--------------------------------------------------------------------------------------
wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.19.238.tar.xz
tar -xf linux-4.19.238.tar.xz
cd linux-4.19.238
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
# CONFIG_XEN_DOM0 = y
# CONFIG_XEN = y
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- Image -j8
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules -j8


--------------------------------------------------------------------------------------
qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 128M \
  -machine type=virt \
  -nographic \
  -kernel /home/hs/qemu-work/linux-4.19.238/arch/arm64/boot/Image \
  -initrd /home/hs/qemu-work/rootfs.img.gz \
  -append "rw root=/dev/ram rdinit=/sbin/init earlyprintk=serial,ttyAMA0 console=ttyAMA0"

--------------------------------------------------------------------------------------

wget ftp://ftp.denx.de/pub/u-boot/u-boot-2019.01.tar.bz2
tar -xf u-boot-2019.01.tar.bz2
cd u-boot-2019.01
make CROSS_COMPILE=aarch64-none-linux-gnu- qemu_arm64_defconfig
make CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
#CONFIG_ARCH_QEMU = y
#CONFIG_TARGET_QEMU_ARM_64BIT = y

make CROSS_COMPILE=aarch64-none-linux-gnu- -j8
--------------------------------------------------------------------------------------

qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 512M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios /home/hs/qemu-work/u-boot-2019.01/u-boot.bin \
  -device loader,file=/home/hs/qemu-work/linux-4.19.238/arch/arm64/boot/Image,addr=0x45000000 \
  -chardev socket,id=qemu-monitor,host=localhost,port=7777,server,nowait,telnet \
  -mon qemu-monitor,mode=readline


  booti 0x45000000 - 0x40000000
  
--------------------------------------------------------------------------------------


    setenv bootargs "rw root=/dev/ram rdinit=/sbin/init earlyprintk=serial,ttyAMA0 console=ttyAMA0"
  
  -chardev socket,id=qemu-monitor,host=localhost,port=7777,server,nowait,telnet \
  -mon qemu-monitor,mode=readline
  -initrd /home/hs/qemu-work/rootfs.img.gz \
  -append "rw root=/dev/ram rdinit=/sbin/init earlyprintk=serial,ttyAMA0 console=ttyAMA0"

--------------------------------------------------------------------------------------
wget https://downloads.xenproject.org/release/xen/4.11.4/xen-4.11.4.tar.gz
tar -xzf xen-4.11.4.tar.gz
cd xen-4.11.4

make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dist-xen -j8

qemu-system-aarch64 \
	-machine virt,gic_version=3 \
	-machine virtualization=true \
	-cpu cortex-a57 \
	-machine type=virt \
	-m 4096 \
	-smp 4 \
	-display none \
	-machine dumpdtb=virt-gicv3.dtb

cd /home/hs/qemu-work
qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 512M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios u-boot-2019.01/u-boot.bin \
  -device loader,file=xen-4.11.4/xen/xen,force-raw=on,addr=0x49000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/Image,addr=0x47000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/virt-gicv3.dtb,addr=0x44000000 \
  -device loader,file=rootfs.img.gz,addr=0x42000000 \
  -chardev socket,id=qemu-monitor,host=localhost,port=7777,server,nowait,telnet \
  -mon qemu-monitor,mode=readline

printf "0x%x\n" $(stat -c %s linux-4.19.238/arch/arm64/boot/Image)
printf "0x%x\n" $(stat -c %s rootfs.img.gz)

fdt addr 0x44000000
fdt resize
fdt resize 512
fdt set /chosen \#address-cells <1>
fdt set /chosen \#size-cells <1>
fdt mknod /chosen module@0
fdt set /chosen/module@0 compatible "xen,linux-image" "xen,multiboot-module"
fdt set /chosen/module@0 reg <0x47000000 0x1228a00>
fdt set /chosen/module@0 bootargs "rw root=/dev/ram rdinit=/sbin/init earlyprintk=serial,ttyAMA0 console=hvc0 earlycon=xenboot"
fdt resize 512
fdt mknod /chosen module@1
fdt set /chosen/module@1 compatible "xen,linux-initrd" "xen,multiboot-module"
fdt set /chosen/module@1 reg <0x42000000 0x111835>
booti 0x49000000 - 0x44000000
  
  
  
  

--------------------------------------------------------------------------------------
  
  
cd /home/hs/qemu-work
qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 128M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios u-boot-2019.01/u-boot.bin \
  -device loader,file=xen-4.11.4/xen/xen,force-raw=on,addr=0x49000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/Image,addr=0x47000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/virt-gicv3.dtb,addr=0x44000000 \
  -drive file=rootfs-jammy/jammy-minimal-arm64.ext4,if=virtio,aio=native,cache.direct=on \
  -chardev socket,id=qemu-monitor,host=localhost,port=7777,server,nowait,telnet \
  -mon qemu-monitor,mode=readline
  
printf "0x%x\n" $(stat -c %s rootfs-jammy/jammy-minimal-arm64.ext4,)

fdt addr 0x44000000
fdt resize
fdt resize 512
fdt set /chosen \#address-cells <1>
fdt set /chosen \#size-cells <1>
fdt mknod /chosen module@0
fdt set /chosen/module@0 compatible "xen,linux-image" "xen,multiboot-module"
fdt set /chosen/module@0 reg <0x47000000 0x1228a00>
fdt set /chosen/module@0 bootargs "rw root=/dev/vda earlyprintk=serial,ttyAMA0 console=hvc0 earlycon=xenboot"
booti 0x49000000 - 0x44000000
  
  
  
  
