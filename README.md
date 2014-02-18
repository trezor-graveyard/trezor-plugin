Bitcoin Trezor Plugin
=====================

1. Project initialization
-------------------------

    git clone https://github.com/slush0/trezor-plugin.git
    git clone git://github.com/firebreath/FireBreath.git firebreath-dev

    cd firebreath-dev
    git submodule update --recursive --init

    mkdir -p projects
    ln -s ../../trezor-plugin projects/BitcoinTrezorPlugin

    cd BitcoinTrezorPlugin
    git submodule update --recursive --init
    cp CMakeLists.trezor-crypto.txt trezor-crypto/CMakeLists.txt


2. Building on Linux
--------------------

* download qcow2 image of Fedora 19 (i386 and/or x86_64)

    http://fedoraproject.org/en/get-fedora-options#cloud

  it will be called something like:

    Fedora-i386-19-20130627-sda.qcow2 or Fedora-x86_64-19-20130627-sda.qcow2

    we'll use $IMAGE from now on

* enable root login (guestfish is from guestfs-tools package)

    sudo guestfish -a $IMAGE -i vi /etc/passwd

    change line

      root:x:0:0:root:/root:/bin/bash
        - to -
      root::0:0:root:/root:/bin/bash

* start QEMU virtual machine

    qemu-system-i386 -hda $IMAGE -m 2048     # (for 32-bit)
      - or -
    qemu-system-x86_64 -hda $IMAGE -m 2048   # (for 64-bit)

* or convert the image to use with VirtualBox and use VirtualBox instead of QEMU

    qemu-img convert -O vdi $IMAGE $IMAGE.vdi

* login as root (no password needed)

* install essential build tools and libraries

    yum install bzip2 cmake gcc-c++ git make wget
    yum install libudev-devel zlib-devel

* download and install openssl from source

    wget http://www.openssl.org/source/openssl-1.0.1e.tar.gz
    tar xfz openssl-1.0.1e.tar.gz
    cd openssl-1.0.1e
    ./config -fPIC no-shared --openssldir=/usr/local
    make
    make install

* download and install protobuf from source

    wget https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.bz2
    tar xfj protobuf-2.5.0.tar.bz2
    cd protobuf-2.5.0
    ./configure --with-pic --disable-shared --enable-static
    make
    make install

* download and install libusbx from source

    wget http://downloads.sourceforge.net/project/libusbx/releases/1.0.17/source/libusbx-1.0.17.tar.bz2
    tar xfj libusbx-1.0.17.tar.bz2
    cd libusbx-1.0.17
    ./configure --with-pic --disable-shared --enable-static
    make
    make install

* setup firebreath and trezor-plugin projects as described in chapter 1

* generate make files and build the project

      cd firebreath-dev
      ./prepmake.sh

      cd build
      make

* binary is located in build/bin/BitcoinTrezorPlugin/npBitcoinTrezorPlugin.so

3. Building on Mac OS X
-----------------------

You will need to have Xcode installed.

    # install dependencies
    brew install cmake protobuf boost

    # generate the makefiles
    ./prepmac.sh projects build \
        -DCMAKE_OSX_ARCHITECTURES=i386 \
        -DWITH_SYSTEM_BOOST=1 \
        -DBoost_USE_STATIC_LIBS=on \
        -DBoost_USE_STATIC_RUNTIME=on

    # build the project
    cd build
    xcodebuild -configuration Debug # or Release

    # symlink to the plugin directory
    ln -s `pwd`/projects/BitcoinTrezorPlugin/Debug/Bitcoin\ Trezor\ Plugin.plugin \
        ~/Library/Internet\ Plug-Ins/

4. Building on Windows
----------------------

Requirements:

- [Visual Studio 2010 Express](http://www.visualstudio.com/en-us/downloads#d-2010-express)
- [Windows Driver Kit 7.1](http://www.microsoft.com/en-us/download/details.aspx?id=11800)
  with the "Build Environments" option enabled, installed to the
  default location
- [CMake 2.8.7 or later](http://www.cmake.org/cmake/resources/software.html)
  binary distribution, allow installer to append CMake binaries to
  PATH
- [Git](http://msysgit.github.io/)
- [Protobuf 2.5.0](https://code.google.com/p/protobuf/downloads/detail?name=protobuf-2.5.0.zip&can=2&q=)
- [Python 2.7](http://python.org/download/)
- [WiX 3.6](http://wix.codeplex.com/releases/view/93929)

Instructions:

1. Build Protobuf

    1. In the `vsprojects` directory, run `extract_includes.bat`
    2. Open the VS solution from the `vsprojects` directory
    3. Switch to preferred configuration (Release/Debug)
    3. Switch the `libprotobuf` project to static runtime `/MT` or `/MTd` (open
       Properties through the right-click menu, then C++, Code Generation, item
       Runtime Library)
    4. Build `libprotobuf` project

2. Build plugin

    1. Generate the VS solution (change the Protobuf path to your installation):

            set PROTOBUF_PATH=C:/Code/protobuf-2.5.0/
            prep.cmd

    2. Open the solution from the generated `build` directory
    3. Switch to preferred configuration
    4. Build the solution
    5. Results of the build are at `build/bin/BitcoinTrezorPlugin/{Debug,Release,...}`
    6. You can either run the installer or register the compiled DLL manually with:

            regsvr32.exe npBitcoinTrezorPlugin.dll

5. Testing
----------

After you build the plugin according to instructions above, you can
open the test page in your web browser:

    pythom -m SimpleHTTPServer
    google-chrome http://localhost:8000/test.html

If you rebuild the plugin you should restart the browser to be sure
you're running the newest version.
