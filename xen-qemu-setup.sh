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
CC=gcc LD=ld ./configure --target-list=aarch64-softmmu
make -j8

QEMU_VER=3.1.1.1
wget http://sources.buildroot.net/qemu/qemu-$QEMU_VER.tar.xz
tar -xvf qemu-$QEMU_VER.tar.xz
cd qemu-$QEMU_VER
CC=gcc LD=ld ./configure --target-list=aarch64-softmmu
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

--------------------------------------------------------------------------------------
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
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- Image -j12
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules -j12

INSTALL_PATH=$(realpath ../linux-4.19.238_install/)  make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  install DESTDIR=../linux-4.19.238_install/
INSTALL_MOD_PATH=$(realpath ../linux-4.19.238_install/) make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  modules_install
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

/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 128M \
  -machine type=virt \
  -nographic \
  -kernel ./linux-5.15.37/arch/arm64/boot/Image \
  -initrd ./rootfs/rootfs.img.gz \
  -append "rw root=/dev/ram rdinit=/sbin/init earlyprintk=serial,ttyAMA0 console=ttyAMA0"


/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 256M \
  -machine type=virt \
  -nographic -no-reboot \
  -kernel linux-5.15.37/arch/arm64/boot/Image \
  -drive file=rootfs/jammy-minimal-arm64.ext4,if=virtio,format=raw,aio=threads,cache=writeback\
  -append "rw root=/dev/vda earlyprintk=serial,ttyAMA0 console=ttyAMA0"

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

wget https://ftp.denx.de/pub/u-boot/u-boot-2022.04.tar.bz2
tar -xjf u-boot-2022.04.tar.bz2
cd u-boot-2022.04
export PATH=$PATH:/home/hs/qemu-work/gcc-aarch64-linux-11.2/bin
CC=gcc LD=ld make CROSS_COMPILE=aarch64-none-linux-gnu- qemu_arm64_defconfig
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
/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 256M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios ./u-boot-2022.04/u-boot.bin \
  -device loader,file=linux-5.15.37/arch/arm64/boot/Image,addr=0x45000000 \
  -drive file=rootfs/jammy-minimal-arm64.ext4,if=virtio,format=raw,aio=threads,cache=writeback
  
setenv bootargs  "rw root=/dev/vda earlyprintk=serial,ttyAMA0 console=ttyAMA0"
booti 0x45000000 - 0x40000000

--------------------------------------------------------------------------------------
wget https://downloads.xenproject.org/release/xen/4.11.4/xen-4.11.4.tar.gz
tar -xzf xen-4.11.4.tar.gz
cd xen-4.11.4

#make clean
#make distclean 
echo "CFLAGS += -mno-outline-atomics" >> config/arm64.mk
make -C xen XEN_TARGET_ARCH=arm64 CROSS_COMPILE==aarch64-none-linux-gnu- CONFIG_EARLY_PRINTK=pl011,0x09000000,115200 CONFIG_DEBUG=y defconfig
make -C xen XEN_TARGET_ARCH=arm64 CROSS_COMPILE==aarch64-none-linux-gnu- CONFIG_EARLY_PRINTK=pl011,0x09000000,115200 CONFIG_DEBUG=y menuconfig
make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- CONFIG_EARLY_PRINTK=pl011,0x09000000,115200 CONFIG_DEBUG=y dist-xen -j 8

make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dist-xen -j8

--------------------------------------------------------------------------------------
XEN_VER="4.11.4"
wget https://downloads.xenproject.org/release/xen/$XEN_VER/xen-$XEN_VER.tar.gz
tar -xzf xen-$XEN_VER.tar.gz
cd xen-$XEN_VER
echo "CFLAGS += -mno-outline-atomics" >> config/arm64.mk

make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dist-xen -j8

--------------------------------------------------------------------------------------
wget https://downloads.xenproject.org/release/xen/4.16.1/xen-4.16.1.tar.gz
tar -xzf xen-4.16.1.tar.gz
cd xen-4.16.1

make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- arm64_defconfig
make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig -C xen
make XEN_TARGET_ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dist-xen -j8


LDFLAGS="${L_LDFLAGS}" \
PKG_CONFIG="${L_PKG_CONFIG}" \
LD="${L_LD} ${L_LDFLAGS}" \
PKG_CONFIG_LIBDIR="${L_PKG_CONFIG_LIBDIR}" \
PKG_CONFIG_SYSROOT_DIR="${L_PKG_CONFIG_SYSROOT_DIR}" \
$L_CONF_EXTRA_FLAGS \

./configure \
CC="aarch64-none-linux-gnu-gcc" \
CXX="aarch64-none-linux-gnu-g++" \
LD="aarch64-none-linux-gnu-ld" \
PYTHON="python" \
--disable-xen \
--enable-tools \
--build=x86_64-linux-gnu \
--host="aarch64-none-linux-gnu" \
--target="aarch64-none-linux-gnu" \
--disable-gtk-doc --disable-gtk-doc-html  --disable-stubdom --disable-ioemu-stubdom --disable-pv-grub \
--disable-xenstore-stubdom --disable-rombios --disable-ocamltools --disable-qemu-traditional --disable-doc \
--disable-docs --disable-documentation --with-xmlto=no --with-fop=no --disable-dependency-tracking --disable-ipv6 --disable-blobs \
--disable-nls --disable-static --enable-shared --with-initddir=/etc/init.d --disable-ocamltools --disable-vnc --disable-gtk --disable-monitors \
--with-extra-qemuu-configure-args="--disable-vnc --disable-gtk --disable-sdl --disable-opengl --disable-werror --disable-libusb"

--------------------------------------------------------------------------------------
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
/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
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

/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
	-machine virt,gic_version=3 \
	-machine virtualization=true \
	-cpu cortex-a57 \
	-machine type=virt \
	-m 4096 \
	-smp 4 \
	-display none \
	-machine dumpdtb=virt-gicv3.dtb

dtc -I dtb -O dts virt-gicv3.dtb > virt-gicv3.dts
sed 's/compatible = "arm,pl061.*/status = "disabled";/g' virt-gicv3.dts > virt-gicv3-edited.dts
dtc -I dts -O dtb virt-gicv3-edited.dts > virt-gicv3.dtb

UBOOT_VER="2022.04"
XEN_VER="4.11.4"
LINUX_VER="5.15.37"
QEMU_VER="6.2.0"

printf "0x%x\n" $(stat -c %s linux-$LINUX_VER/arch/arm64/boot/Image)

/home/hs/work/xen-qemu/qemu-3.1.0/aarch64-softmmu/qemu-system-aarch64 \
/home/hs/work/xen-qemu/qemu-3.1.1.1/aarch64-softmmu/qemu-system-aarch64 \
/home/hs/work/xen-qemu/qemu.git/build/qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 512M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios u-boot-$UBOOT_VER/u-boot.bin \
  -device loader,file=xen-$XEN_VER/xen/xen,force-raw=on,addr=0x49000000 \
  -device loader,file=linux-$LINUX_VER/arch/arm64/boot/Image,addr=0x46000000 \
  -device loader,file=virt-gicv3.dtb,addr=0x44000000 \
  -drive file=rootfs/jammy-minimal-arm64.ext4,if=virtio,format=raw,aio=threads,cache=writeback
  


fdt addr 0x44000000
fdt resize
fdt resize 512
fdt set /chosen \#address-cells <1>
fdt set /chosen \#size-cells <1>
fdt mknod /chosen module@0
fdt set /chosen/module@0 compatible "xen,linux-image" "xen,multiboot-module"
fdt set /chosen/module@0 reg <0x46000000 0x200e200>
fdt set /chosen/module@0 bootargs "rw root=/dev/vda earlyprintk=serial,ttyAMA0 console=hvc0 earlycon=xenboot"
fdt resize 512
booti 0x49000000 - 0x44000000
  

  
  

--------------------------------------------------------------------------------------
  
  
cd /home/hs/qemu-work
qemu-system-aarch64 \
  -machine virt,gic_version=3 \
  -machine virtualization=true \
  -cpu cortex-a57 -smp 4 -m 128M \
  -machine type=virt \
  -nographic -no-reboot \
  -bios u-boot-2022.04/u-boot.bin \
  -device loader,file=xen-4.16.1/xen/xen,force-raw=on,addr=0x49000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/Image,addr=0x47000000 \
  -device loader,file=linux-4.19.238/arch/arm64/boot/virt-gicv3.dtb,addr=0x44000000 \
  -drive file=rootfs-jammy/jammy-minimal-arm64.ext4,if=virtio,format=raw,cache=none \
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
  
  
  
  
