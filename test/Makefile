#author koshy george, @2014

#for building debug configuraion, call make by 'make DEBUG=1'
DEBUG ?= 0
GIT_ROOT=./..
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
else
	CONFIG=RELEASE
	FORGIVING-CXXFLAGS=-Wno-unused-variable -Wno-reorder  -D NDEBUG
	OPENCV-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)
	OPENCV-LIBS-DIR = /usr/local/lib
	#OPENCV-LIBS-DIR = $(shell /usr/local/bin/pkg-config --libs opencv | dirname `awk '{ print $$1 }'`)
	GTEST-LIBS-DIR = $(GIT_ROOT)/thirdparty/gtest-1.7.0/lib/release
	GTEST-INCLUDES = -I$(GIT_ROOT)/thirdparty/gtest-1.7.0/include
	EIGEN-LIBS = $(shell /usr/local/bin/pkg-config --libs eigen3)
	EIGEN-INCLUDES=$(shell /usr/local/bin/pkg-config --cflags eigen3)
	KGEORGECV-INCLUDES= -I$(GIT_ROOT)/include
endif


OPENCV-LIBS=\
        -L/usr/local/lib -lopencv_core -lopencv_highgui -lippicv \
        -F /System/Library/Frameworks   -framework OpenGL -framework AGL

#OPENCV-LIBS=\
#        /usr/local/lib/libopencv_core.a \
#        /usr/local/lib/libopencv_calib3d.a \
#        /usr/local/lib/libopencv_features2d.a \
#        /usr/local/lib/libopencv_highgui.a \
#        /usr/local/lib/libopencv_imgproc.a \
#        -F /System/Library/Frameworks   -framework OpenGL -framework AGL
#OPENCV-LIBS=\
#	$(OPENCV-LIBS-DIR)/libopencv_calib3d.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_contrib.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_core.dylib   \
#	$(OPENCV-LIBS-DIR)/libopencv_features2d.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_flann.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_gpu.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_highgui.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_imgproc.dylib  \
#	$(OPENCV-LIBS-DIR)/libopencv_legacy.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_ml.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_nonfree.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_objdetect.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_ocl.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_photo.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_stitching.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_superres.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_ts.a \
#	$(OPENCV-LIBS-DIR)/libopencv_video.dylib \
#	$(OPENCV-LIBS-DIR)/libopencv_videostab.dylib \


GTEST-LIBS=\
	$(GTEST-LIBS-DIR)/libgtest.a \
	$(GTEST-LIBS-DIR)/libgtest_main.a



CXX=clang++
CXXFLAGS=-g  -v -Wall -Wno-deprecated $(FORGIVING-CXXFLAGS) -std=c++11 -D WITH_OPENGL  $(OPENCV-INCLUDES) $(EIGEN-INCLUDES) $(KGEORGECV-INCLUDES)
LDFLAGS= $(OPENCV-LIBS) $(EIGEN-LIBS)

BUILD-DIR=build
MAIN=$(addprefix $(BUILD-DIR)/, main.o )
OBJS=$(addprefix $(BUILD-DIR)/, spotIt.o)
PROG=$(BUILD-DIR)/spotIt


TESTOBJS=$(addprefix $(BUILD-DIR)/, testMain.o)
TESTPROG=$(BUILD-DIR)/testMain

all:  $(TESTPROG)

#order-only-prerequisites
#order-only-prerequisites
$(TESTOBJS) : | $(BUILD-DIR)


$(TESTPROG): $(TESTOBJS)
	@echo linking $(CONFIG) configuration for test, LDFLAGS is $(LDFLAGS)
	$(CXX) -v -o $(TESTPROG)  $(TESTOBJS) $(LDFLAGS) $(GTEST-LIBS)


$(BUILD-DIR)/%.o: %.cpp
	@echo compiling $(CONFIG) configuration for test
	$(CXX) -c $(CXXFLAGS) $(GTEST-INCLUDES) -o $@ $<


$(BUILD-DIR):
	mkdir -p $(BUILD-DIR)

clean:
	rm -rf build
