#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
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

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi

if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    
    # Deep clean the kernel build tree, removing the .config file w/ any existing configurations
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    
    #defconfig build for QEMU "virt" arm dev board
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    
    #build vmlinux
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    
    #build any kernel modules - instructions appear to say not to do this
    #make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}modules
    
    #build the device tree
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
#JDE: This wasn't here as a TODO: for some reason. The instructions never referenced doing it, either
# yet the start-qemu-terminal.sh expects Image to be placed here.
cd "$OUTDIR/linux-stable/arch/$ARCH/boot"
cp Image "$OUTDIR"
cp Image.gz "$OUTDIR"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir rootfs
cd rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

cd "$OUTDIR/rootfs"

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs

SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
cp "$SYSROOT/lib/ld-linux-aarch64.so.1" lib
cp "$SYSROOT/lib64/libm.so.6" lib64
cp "$SYSROOT/lib64/libresolv.so.2" lib64
cp "$SYSROOT/lib64/libc.so.6" lib64

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# TODO: Clean and build the writer utility
cd "$FINDER_APP_DIR"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
TARGET_HOME="$OUTDIR/rootfs/home"
cp writer "$TARGET_HOME"
cp finder.sh "$TARGET_HOME"
cp finder-test.sh "$TARGET_HOME"
cp autorun-qemu.sh "$TARGET_HOME"

mkdir -p "$TARGET_HOME/conf"
cp conf/username.txt "$TARGET_HOME/conf"
cp conf/assignment.txt "$TARGET_HOME/conf"


# TODO: Chown the root directory
# JDE - this step is unnecessary, and the instructional materials make no mention of it.
# instead, the Embedded Linux Programming book states that the --owner root:root option used with cpio
# in a later step satisfies the ownership requirements.

# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio

cd ${OUTDIR}
gzip -f initramfs.cpio
