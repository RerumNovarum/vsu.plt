#!/bin/bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/.local -Dvsunum_DIR=~/.local/share/libvsu/ .. \
&& make \
&& make test \
&& make install
