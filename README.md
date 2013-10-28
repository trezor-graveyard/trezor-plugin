HOWTO
=====

Project cloning and initialization:

    git clone git@github.com:slush0/trezor-plugin.git
    git clone git://github.com/firebreath/FireBreath.git firebreath-dev

    cd firebreath-dev
    git submodule update --recursive --init

    mkdir -p projects
    ln -s `pwd`/../trezor-plugin projects/BitcoinTrezorPlugin

After you build the plugin according to instructions below, you can
open the test page in your web browser:

    pythom -m SimpleHTTPServer
    google-chrome http://localhost:8000/test.html

If you rebuild the plugin you should restart the browser to be sure
you're running the newest version.

Building on Linux
-----------------

Instructions for Ubuntu 13.04 (raring):

    # install dependencies
    sudo apt-get install python-software-properties
    sudo add-apt-repository -y ppa:chris-lea/protobuf
    sudo apt-get update
    sudo apt-get install \
        build-essential cmake git \
        libprotobuf-dev libssl-dev libboost1.53-all-dev libusb-1.0-0-dev

    # generate the makefiles, rerun if you add any library/header
    ./prepmake.sh projects build \
        -DWITH_SYSTEM_BOOST=1 \
        -DBoost_USE_STATIC_LIBS=on \
        -DBoost_USE_STATIC_RUNTIME=on

    # build the project
    cd build
    make

    # symlink to the plugin directory
    mkdir -p ~/.mozilla/plugins
    ln -s `pwd`/bin/BitcoinTrezorPlugin/npBitcoinTrezorPlugin.so ~/.mozilla/plugins/

Building on Mac OS X
--------------------

You will need to have Xcode installed.

    # install dependencies
    brew install cmake protobuf boost

    # generate the makefiles
    ./prepmac.sh projects build \
        -DWITH_SYSTEM_BOOST=1 \
        -DBoost_USE_STATIC_LIBS=on \
        -DBoost_USE_STATIC_RUNTIME=on

    # build the project
    cd build
    xcodebuild

    # symlink to the plugin directory
    ln -s `pwd`/projects/BitcoinTrezorPlugin/Debug/Bitcoin\ Trezor\ Plugin.plugin \
        ~/Library/Internet\ Plug-Ins/

Building on Windows
-------------------

Requirements:

- [Visual Studio 2010 Express](http://www.microsoft.com/express/Downloads/)
- [Windows Driver Kit 7.1](http://www.microsoft.com/express/Downloads/)
  with the "Build Environments" option enabled, installed to the
  default location
- [CMake 2.8.7 or later](http://www.cmake.org/cmake/resources/software.html)
  binary distribution, allow installer to append CMake binaries to
  PATH
- [Git](http://msysgit.github.io/)
- [Protobuf 2.5.0](https://code.google.com/p/protobuf/downloads/detail?name=protobuf-2.5.0.zip&can=2&q=)
- [Python 2.7](http://python.org/download/)

Instructions:

1. Open protobuf solution from the `vsprojects` directory and switch
   it to use static runtime (`/MT`) at least for the `libprotobuf`
   project.
2. Build protobuf.
3. Generate the VS solution. Note it doesn't use system boost yet.

        set PROTOBUF_PATH=C:/Code/protobuf-2.5.0/
        prep2010.cmd projects build ^
            "-DBoost_USE_STATIC_LIBS=on" ^
	        "-DBoost_USE_STATIC_RUNTIME=on" ^
	        "-DPROTOBUF_SRC_ROOT_FOLDER=%PROTOBUF_PATH%" ^
	        "-DPROTOBUF_INCLUDE_DIR=%PROTOBUF_PATH%/vsprojects/include"

4. Build the solution from the generated `build` directory.
5. Register the compiled DLL:

        cd build/bin/BitcoinTrezorPlugin/Debug
        regsvr32.exe npBitcoinTrezorPlugin.dll
