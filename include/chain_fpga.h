#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include "xcl2.hpp"
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <limits.h>

using namespace std;

// Parameters of the hardware architecture (Important: don't change the values below unless you recompile hardware code (src/fpga/minimap2_opencl.cl))
#define NUM_HW_KERNELS 1
#define TRIPCOUNT_PER_SUBPART 128
#define MAX_SUBPARTS 8
#define FPGA_MAX_TRIPCOUNT (TRIPCOUNT_PER_SUBPART * MAX_SUBPARTS)

#define EXTRA_ELEMS 0  // added to temporarily fix the issue with parallel execution of OpenCL hardware kernels on Intel devices
                       // (i.e. all input/output arrays used in hardware chaining are filled with EXTRA_ELEMS no. of elements)

// #define DEBUG_HW    // to print out steps in hardware processing

// Size of the OpenCL buffers used for data transfer
#define DEVICE_MAX_N 332000000
#define BUFFER_MAX_N (DEVICE_MAX_N / 2)
#define BUFFER_N (BUFFER_MAX_N / 32)

#define STRING_BUFFER_LEN 1024

void perform_core_chaining_on_fpga(int64_t, int32_t, int32_t, int32_t, int32_t, float, anchor_t*, int32_t*, int32_t*, unsigned char*, int64_t, int32_t);
bool hardware_init(long);
void cleanup();
void checkError(cl_int err, const string message);