HOWTO
-----

    mkdir ~/.mozilla/plugins

System requirements
    
    sudo apt-get install git cmake libprotobuf-dev libboost-random-dev libusb-1.0-0-dev

Project cloning and initialization

    git clone git@github.com:slush0/bitkey-plugin.git
    git clone git://github.com/firebreath/FireBreath.git firebreath-dev
    cd firebreath-dev
    git submodule update --recursive --init
    mkdir projects
    ln -s `pwd`/../bitkey-plugin projects/BitcoinTrezorPlugin

If you add any new library/header requirement you should rerun `prepmake`

    ./prepmake.sh

(Re)building project

    cd build
    make

After first time you can create symlink to you mozilla plugins directory

    ln -s `pwd`/bin/BitcoinTrezorPlugin/npBitcoinTrezorPlugin.so ~/.mozilla/plugins/

The you can test your plugin in web browser.
If you update and rebuild your code you should restart plugin (or browser).
Only page refreshing can't help you, you'll run *old* plugin version.

    firefox ../projects/BitcoinTrezorPlugin/test.html
