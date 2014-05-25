#author koshy george, @2014

#for building debug configuraion, call make by 'make DEBUG=1'
DEBUG ?= 1
GIT_ROOT=./../..

PYTHON_VERSION=2.7
#PYTHON_INCLUDE=/usr/include/python$(PYTHON_VERSION)
PYTHON_INCLUDE=/System/Library/Frameworks/Python.framework/Versions/$(PYTHON_VERSION)/include/python$(PYTHON_VERSION)/

BOOST_INC_DIR=$(GIT_ROOT)/thirdparty/boost_1_55_0/include
BOOST_LIB_DIR=$(GIT_ROOT)/thirdparty/boost_1_55_0/lib
 
# compile mesh classes

ifeq ($(DEBUG), 1)
	CONFIG=DEBUG
	#
	#change this to point to your debug build of opencv
	#
	OPENCV_DEBUG_INSTALL_DIR=/Users/kgeorge/Documents/projects/opencv-2.4.8/build/debug
	FORGIVING-CXXFLAGS=-Wunused-variable -Wno-reorder -D DEBUG -g3
	OPENCV-INCLUDES = /usr/local/include  $(OPENCV_DEBUG_INSTALL_DIR)/include
	OPENCV-LIBS-DIR = $(OPENCV_DEBUG_INSTALL_DIR)/lib
	# for now just use the release config
	GTEST-LIBS-DIR = $(GIT_ROOT)/thirdparty/gtest-1.7.0/lib/release
	GTEST-INCLUDES = -I$(GIT_ROOT)/thirdparty/gtest-1.7.0/include
	EIGEN-LIBS = $(shell /usr/local/bin/pkg-config --libs eigen3)
	EIGEN-INCLUDES=$(shell /usr/local/bin/pkg-config --cflags eigen3)
	KGEORGECV-INCLUDES= -I$(GIT_ROOT)/include
	BOOST-INCLUDES= -I$(BOOST_INC_DIR)
	BOOST-LIBS=$(BOOST_LIB_DIR)/libboost_python.dylib $(BOOST_LIB_DIR)/numpy/libboost_numpy.dylib
else
	CONFIG=RELEASE
	FORGIVING-CXXFLAGS=-Wno-unused-variable -Wno-reorder  -D NDEBUG
	OPENCV-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)
	OPENCV-LIBS-DIR = $(shell /usr/local/bin/pkg-config --libs opencv | dirname `awk '{ print $$1 }'`)
	GTEST-LIBS-DIR = $(GIT_ROOT)/thirdparty/gtest-1.7.0/lib/release
	GTEST-INCLUDES = -I$(GIT_ROOT)/thirdparty/gtest-1.7.0/include
	EIGEN-LIBS = $(shell /usr/local/bin/pkg-config --libs eigen3)
	EIGEN-INCLUDES=$(shell /usr/local/bin/pkg-config --cflags eigen3)
	KGEORGECV-INCLUDES= -I$(GIT_ROOT)/include
	BOOST-INCLUDES= -I$(BOOST_INC_DIR)
	BOOST-LIBS=$(BOOST_LIB_DIR)/libboost_python.dylib $(BOOST_LIB_DIR)/numpy/libboost_numpy.dylib
endif


OPENCV-LIBS=\
	$(OPENCV-LIBS-DIR)/libopencv_calib3d.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_contrib.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_core.dylib   \
	$(OPENCV-LIBS-DIR)/libopencv_features2d.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_flann.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_gpu.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_highgui.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_imgproc.dylib  \
	$(OPENCV-LIBS-DIR)/libopencv_legacy.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_ml.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_nonfree.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_objdetect.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_ocl.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_photo.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_stitching.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_superres.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_ts.a \
	$(OPENCV-LIBS-DIR)/libopencv_video.dylib \
	$(OPENCV-LIBS-DIR)/libopencv_videostab.dylib \
	-F /System/Library/Frameworks   -framework OpenGL -framework AGL

GTEST-LIBS=\
	$(GTEST-LIBS-DIR)/libgtest.a \
	$(GTEST-LIBS-DIR)/libgtest_main.a



CXX=clang++
CXXFLAGS=-g -Wall -Wno-deprecated $(FORGIVING-CXXFLAGS) -std=c++11 $(BOOST-INCLUDES) -I$(PYTHON_INCLUDE) $(KGEORGECV-INCLUDES)
LDFLAGS= -shared $(BOOST-LIBS) -lpython$(PYTHON_VERSION)

BUILD-DIR=build
#MAIN=$(addprefix $(BUILD-DIR)/, main.o )
OBJS=$(addprefix $(BUILD-DIR)/, hogc.o)
PROG=$(BUILD-DIR)/hogc


#TESTOBJS=$(addprefix $(BUILD-DIR)/, testMain.o)
#TESTPROG=$(BUILD-DIR)/testMain

all:  $(PROG)

#order-only-prerequisites
#order-only-prerequisites
$(OBJS) : | $(BUILD-DIR)


$(PROG): $(OBJS)
	@echo $(CXX) -v -o $(PROG)  $(OBJS) $(LDFLAGS) 
	$(CXX) -v -o $(PROG).so  $(OBJS) $(LDFLAGS)


$(BUILD-DIR)/%.o: %.cpp
	@echo compiling $(CONFIG) configuration for test
	$(CXX) -c $(CXXFLAGS)  -o $@ $<


$(BUILD-DIR):
	mkdir -p $(BUILD-DIR)

clean:
	rm -rf build

ship:
	cp  $(PROG).so $(GIT_ROOT)/lib
