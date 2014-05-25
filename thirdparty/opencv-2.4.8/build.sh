#!/bin/sh
cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_NEW_PYTHON_SUPPORT=ON \
 -D PYTHON_PACKAGES_PATH=/Library/Python/2.7/site-packages -D BUILD_EXAMPLES=ON  -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS="-std=c++11 -stdlib=libc++" ./..

