# efuse_control
lib2.0을 사용한 ODROID-M1S efuse r/w

### Image used for testing.
* smb 192.168.0.197://HK_197_SMBRK3566/ubuntu-20.04-server-odroidm1s-20230911.img.xz

### Install package
* vim, build-essentail, git, overlayroot, python3, python3-pip, cups, cups-bsd, pkg-config, ssh, ethtool, nmap 
* python3 -m pip install aiohttp asyncio

### Use the submodule.
* Add the lib_mac, lib_nlp, lib_efuse submodule to the efuse_control repository.
```
root@odroid:~/efuse_control# git submodule add https://github.com/charles-park/lib_mac
Cloning into '/root/efuse_control/lib_mac'...
...

root@odroid:~/efuse_control# git commit -am "lib_mac submodule add"
...

root@odroid:~/efuse_control# git push origin
...

root@odroid:~/efuse_control# 

```

* Clone the reopsitory with submodule.
```
root@odroid:~# git clone --recursive https://github.com/charles-park/efuse_control

or

root@odroid:~# git clone https://github.com/charles-park/efuse_control
root@odroid:~# cd efuse_control
root@odroid:~/efuse_control# git submodule init
root@odroid:~/efuse_control# git submodule update
```
### Overlay root
* overlay-root enable
```
root@odroid:~# update-initramfs -c -k $(uname -r)
update-initramfs: Generating /boot/initrd.img-4.9.277-75
root@odroid:~#
root@odroid:~# mkimage -A arm64 -O linux -T ramdisk -C none -a 0 -e 0 -n uInitrd -d /boot/initrd.img-$(uname -r) /boot/uInitrd 
Image Name:   uInitrd
Created:      Wed Feb 23 09:31:01 2022
Image Type:   AArch64 Linux RAMDisk Image (uncompressed)
Data Size:    13210577 Bytes = 12900.95 KiB = 12.60 MiB
Load Address: 00000000
Entry Point:  00000000
root@odroid:~#

overlayroot.conf 파일의 overlayroot=””를 overlayroot=”tmpfs”로 변경합니다.
vi /etc/overlayroot.conf
overlayroot_cfgdisk="disabled"
overlayroot="tmpfs"
```
* overlay-root modified/disable  
```
[get write permission]
odroid@hirsute-server:~$ sudo overlayroot-chroot 
INFO: Chrooting into [/media/root-ro]
root@hirsute-server:/# 

[disable overlayroot]
overlayroot.conf 파일의 overlayroot=”tmpfs”를 overlayroot=””로 변경합니다.
vi /etc/overlayroot.conf
overlayroot_cfgdisk="disabled"
overlayroot=""

```
