#author koshy george, @2014

#for building debug configuraion, call make by 'make DEBUG=1'
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CONFIG=DEBUG
	#
	#change this to point to your debug build of opencv
	#
	OPENCV_DEBUG_INSTALL_DIR=/Users/kgeorge/Documents/projects/opencv-2.4.8/build/debug
	FORGIVING-CXXFLAGS=-Wno-reorder -D DEBUG -g3
	OPENCV-INCLUDES = /usr/local/include  $(OPENCV_DEBUG_INSTALL_DIR)/include
	OPENCV-LIBS-DIR = $(OPENCV_DEBUG_INSTALL_DIR)/lib
else
	CONFIG=RELEASE
	FORGIVING-CXXFLAGS=-Wno-reorder  -D NDEBUG
	OPENCV-INCLUDES = $(shell /usr/local/bin/pkg-config --cflags opencv)
	OPENCV-LIBS-DIR = $(shell /usr/local/bin/pkg-config --libs opencv | dirname `awk '{ print $$1 }'`)
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



CXX=clang++
CXXFLAGS=-g -Wall -Wno-deprecated $(FORGIVING-CXXFLAGS) -std=c++11 -D WITH_OPENGL  $(OPENCV-INCLUDES)
LDFLAGS= $(OPENCV-LIBS)

BUILD-DIR=build
OBJS=$(addprefix $(BUILD-DIR)/, main.o spotIt.o)
PROG=$(BUILD-DIR)/spotIt



all: $(PROG)

#order-only-prerequisites
$(OBJS): | $(BUILD-DIR)

$(PROG): $(OBJS)
	$(CXX) -v -o $(PROG) $(OBJS) $(LDFLAGS)

$(BUILD-DIR)/%.o: %.cpp
	@echo compiling $(CONFIG) configuration
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD-DIR):
	mkdir -p $(BUILD-DIR)
clean:
	rm $(OBJS); rm $(PROG)
