#!/bin/sh

#Please correct the python install path below
MY_PYTHON_PACKAGES_PATH=/Users/kgeorge/my-env/lib/python2.7/site-packages
cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_NEW_PYTHON_SUPPORT=ON \
 -D PYTHON_PACKAGES_PATH=$MY_PYTHON_PACKAGES_PATH -DWITH_IPP=ON  -D BUILD_SHARED_LIBS=ON  -D BUILD_EXAMPLES=OFF  -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS="-std=c++11 -stdlib=libc++" ./..



