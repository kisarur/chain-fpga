# chainFPGA
*chainFPGA* is a library API designed for FPGA-based chaining step acceleration, which is a crucial computational step in many third-generation bioinformatics sequence alignment software. This library API provides a user-friendly interface to integrate the FPGA-based chaining step accelerator (originaly used in [Minimap2_FPGA](https://github.com/kisarur/minimap2_fpga_opencl)) with other bioinformatics tools without requiring users to engage in internal hardware development.

## Building

1. Install "Intel® PAC with Intel® Arria® 10 GX FPGA Acceleration Stack Version 1.2.1" on the system.

2. Use the commands below to download the GitHub repo and setup the environment (you may need to update the variables defined in `scripts/init_env.sh`, if they're not already pointing to the correct paths in your system).
    ```
    git clone git@github.com:kisarur/chainFPGA.git
    cd chainFPGA
    source scripts/init_env.sh
    ``` 

3. [OPTIONAL] If you use "Intel® PAC with Intel® Arria® 10 GX FPGA" board, for which our accelerator implementation (on this branch) is optimized, it is recommended that you let the library use the already built hardware binary included with this repo at `lib/minimap2_opencl.aocx`. However, if you want to build this binary from source (in `src/fpga/minimap2_opencl.cl`), you can use the command below (note that this process can take hours to complete).
    ```
    ./scripts/generate_fpga_binary.sh
    ```

4. To build the host side (software) of the libray API, use the command below. 
    ```
    make
    ```

## Usage

Include `<chain_fpga.h>` in your C/C++ program and call the *chainFPGA* API functions. Use the command below to compile your program and link it against *chainFPGA*. 
```
g++ [OPTIONS] -I path/to/chainFPGA/include -I path/to/chainFPGA/include/opencl -L path/to/chainFPGA/lib -lchainfpga  
```

Please note that `[OPTIONS]` should contain other relevant Intel FPGA and OpenCL headers/libraries setup in your system. To help with the compilation process, we have created a `Makefile` that includes all required compilation commands. Please see the examples under `examples/` which also contain this `Makefile`.  

## API Functions

The following C/C++ API functions are supported by the *chainFPGA* library.

```c
bool hardware_init(long buf_size);
```
This function initializes the OpenCL hardware resources (buffers, kernels, etc.) for hardware chaining tasks. The size of input/output buffers (decides the maximum no. of anchors that can be processed by any chaining task, and is limited by FPGA device's global memory) is specified as an argument. Call this function once before any `perform_core_chaining_on_fpga()` function call.

```c
void perform_core_chaining_on_fpga(int64_t n, int32_t max_dist_x, int32_t max_dist_y, int32_t bw, int32_t q_span, float avg_qspan_scaled, anchor_t* a, int32_t* f, int32_t* p, unsigned char* num_subparts, int64_t total_subparts, int32_t kernel_id);
```
This is the key function in chainFPGA that takes in the anchor data and other parameters corresponding to the chaining task and processes it (transfer the inputs to FPGA, perform chaining on FPGA, transfer the output to host) on the FPGA-based hardware accelerator. The example in `examples/hardware/` can be used as a guide to know how the arguments to this function should be set properly.

```c
void cleanup();
```
This function cleans up the OpenCL hardware resources allocated by `hardware_init()`. This should be called only once in your program after all the `perform_core_chaining_on_fpga()` calls are completed.

## Acknowledgment

The chaining algorithm used in *chainFPGA* was inspired by the core chaining computation of `mm_chain_dp` function in [Minimap2](https://github.com/lh3/minimap2).