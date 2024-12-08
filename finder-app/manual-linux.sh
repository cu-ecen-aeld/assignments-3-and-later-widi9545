#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p /tmp/aeld

cd /tmp/aeld
if [ ! -d "/tmp/aeld/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN /tmp/aeld"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e $/tmp/aeld/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    #make ARCH=${ARCH} CROSS_COMPILE=$CROSS_COMPILE mrproper
    make -j 9 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
    make -j 9 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all
    make -j 9 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs
fi

echo "Adding the Image in outdir"


echo "Creating the staging directory for the root filesystem"
cd "/tmp/aeld"
if [ -d "/tmp/aeld/rootfs" ]
then
	echo "Deleting rootfs directory at $/tmp/aeld/rootfs and starting over"
    sudo rm  -rf /tmp/aeld/rootfs
fi

# TODO: Create necessary base directories
mkdir /tmp/aeld/rootfs
cd /tmp/aeld/rootfs
mkdir -p bin dev etc home lib lib64 root proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log 

echo $PWD
cp /tmp/aeld/linux-stable/arch/arm64/boot/Image /tmp/aeld/rootfs

cd "/tmp/aeld"
if [ ! -d "/tmp/aeld/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} 
make CONFIG_PREFIX=/tmp/aeld/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

cd /tmp/aeld/rootfs

cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 /tmp/aeld/rootfs/lib64
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 /tmp/aeld/rootfs/lib
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libm.so.6 /tmp/aeld/rootfs/lib64
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libm.so.6 /tmp/aeld/rootfs/lib
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 /tmp/aeld/rootfs/lib64
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 /tmp/aeld/rootfs/lib
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libc.so.6 /tmp/aeld/rootfs/lib64
cp /home/widi9545/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libc.so.6 /tmp/aeld/rootfs/lib

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"



# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 622 dev/tty1 c 4 2 
sudo mknod -m 622 dev/console c 5 1 

# TODO: Clean and build the writer utility
cp /home/widi9545/assignment-1-widi9545/finder-app/writer.c /tmp/aeld/rootfs/home
cp /home/widi9545/assignment-1-widi9545/finder-app/start-qemu-terminal.sh /tmp/aeld/rootfs/home
cp /home/widi9545/assignment-1-widi9545/finder-app/start-qemu-app.sh /tmp/aeld/rootfs/home
cp /home/widi9545/assignment-1-widi9545/finder-app/autorun-qemu.sh /tmp/aeld/rootfs/home

cp -r /home/widi9545/assignment-1-widi9545/conf /tmp/aeld/rootfs/home
cp /home/widi9545/assignment-1-widi9545/finder-app/finder-test.sh /tmp/aeld/rootfs/home
cp /home/widi9545/assignment-1-widi9545/finder-app/finder.sh /tmp/aeld/rootfs/home

chmod -R 777 /tmp/aeld/rootfs/home

cd /tmp/aeld/rootfs/home
${CROSS_COMPILE}gcc writer.c -o writer 

# TODO: Chown the root directory
cd /tmp/aeld/rootfs
echo "we are here $PWD"

sudo find . | sudo cpio -H newc -ov --owner root:root > initramfs.cpio
# TODO: Create initramfs.cpio.gz
gzip -f initramfs.cpio
