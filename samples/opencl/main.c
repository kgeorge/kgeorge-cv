
#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

#include "kernel.c.h"


#define NUM_VALUES 1024


int main( int argc, char **argv) {

    dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
    if(queue == NULL) {
        queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }
    dispatch_release(queue);
    return 0;
}