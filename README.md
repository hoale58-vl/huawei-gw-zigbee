# Clone openwrt repo

```
git clone https://github.com/h4ck3rm1k3/OpenWrt-Firefly-SDK openwrt
cd mt7688a && tar -jxvf buildroot-gcc463.tar.bz2
mv buildroot-gcc463/usr/* ../openwrt/package
cd ..
```

# Install prerequisite

```
sudo yum install ncurses-devel glibc-devel glibc-static  
```

# Include zigbee src-link to feeds conf

```
cd ..
echo src-link zigbee $(pwd)/zigbee-feed >> openwrt/feeds.conf.default
cd openwrt
./scripts/feeds update zigbee
./scripts/feeds install -a -p zigbee
make menuconfig
```

# Build applications

```
make package/iot/compile
```

# Compile

- On PC
```
gcc -I$(pwd) -pthread -o main serial.c main.c
```

- Cross Compile
```
mipsel-openwrt-linux-uclibc-gcc \
    -Os -pipe -mno-branch-likely -mips32r2 -mtune=24kec -mdsp \
    -fno-caller-saves -fhonour-copts \
    -Wno-error=unused-but-set-variable -msoft-float \
    -I/home/black/zigbee/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/iot-1.0 \
    -pthread \
    -o /home/black/zigbee/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/iot-1.0/iot \
    /home/black/zigbee/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/iot-1.0/main.c \
    /home/black/zigbee/openwrt/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/iot-1.0/serial.c
```