#include <assert.h>
#include <math.h>
#include <cstring>
#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Parameters of the hardware architecture (Important: don't change the values below unless you recompile hardware code (device/minimap2_opencl.cl))
#define NUM_HW_KERNELS 1
#define TRIPCOUNT_PER_SUBPART 64
#define MAX_SUBPARTS 8
#define FPGA_MAX_TRIPCOUNT (TRIPCOUNT_PER_SUBPART * MAX_SUBPARTS)

#define EXTRA_ELEMS 0  // added to temporarily fix the issue with parallel execution of OpenCL hardware kernels on Intel devices
                       // (i.e. all input/output arrays used in hardware chaining are filled with EXTRA_ELEMS no. of elements)

// #define DEBUG_HW          // chain_hardware.cpp (to print out steps in hardware processing)

// Size of the OpenCL buffers used for data transfer
#define DEVICE_MAX_N 332000000
#define BUFFER_MAX_N (DEVICE_MAX_N / 2)
#define BUFFER_N (BUFFER_MAX_N / 32)

#define STRING_BUFFER_LEN 1024

void perform_core_chaining_on_fpga(int64_t, int32_t, int32_t, int32_t, int32_t, float, anchor_t*, int32_t*, int32_t*, unsigned char*, int64_t, int32_t);
bool hardware_init(long);
void cleanup();