# chain-fpga
*chain-fpga* is a library API designed for FPGA-based chaining step acceleration, which is a crucial computational step in many third-generation bioinformatics sequence alignment software. This library API provides a user-friendly interface to integrate the FPGA-based chaining step accelerator (originaly used in [minimap2-fpga](https://github.com/kisarur/minimap2-fpga)) with other bioinformatics tools without requiring users to engage in internal hardware development.

## Building

1. Create an [Amazon EC2 F1 instance](https://aws.amazon.com/ec2/instance-types/f1/)  loaded with [FPGA Developer AMI](https://aws.amazon.com/marketplace/pp/prodview-gimv3gqbpe57k) (AWS *f1.2xlarge* instance loaded with *FPGA Developer AMI Version 1.12.1* was used in this work).

2. Use the commands below to download the [AWS EC2 FPGA Development Kit](https://github.com/aws/aws-fpga) and setup the runtime environment.
```
git clone https://github.com/aws/aws-fpga.git $AWS_FPGA_REPO_DIR
source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh
```

2. Use the commands below to download this work's GitHub repo and change to project directory.
```
git clone git@github.com:kisarur/chain-fpga.git
cd chain-fpga
``` 

3. [OPTIONAL] If you use Xilinx UltraScale+ VU9P based FPGA board available on AWS EC2 F1 instance, for which our accelerator implementation (on this branch) is optimized, it is recommended that you use the already built FPGA hardware binary (also called AFI - Amazon FPGA Image) included with this repo at `lib/minimap2_opencl.awsxclbin`. However, if you want to build this AFI from source (in `src/fpga/minimap2_opencl.cl`), you can use the guide available at https://github.com/aws/aws-fpga/tree/master/Vitis (as recommended on the guide, using a non-F1 EC2 compute instance for this time-consuming hardware build step will help minimize costs).

4. To build the host side (software) of the libray API, use the command below. 
    ```
    make host
    ```

## Usage

Include `<chain_fpga.h>` in your C/C++ program and call the *chain-fpga* API functions. Use the command below to compile your program and link it against *chain-fpga*. 
```
g++ [OPTIONS] -I path/to/chain-fpga/include -I path/to/chain-fpga/include/opencl -L path/to/chain-fpga/lib -lchainfpga  
```

Please note that `[OPTIONS]` should contain other relevant Xilinx FPGA and OpenCL headers/libraries setup in your system. To help with the compilation process, we have created a `Makefile` that includes all required compilation commands. Please see the examples under `examples/` which also contain this `Makefile`.  

## API Functions

The following C/C++ API functions are supported by the *chain-fpga* library.

```c
bool hardware_init(long buf_size);
```
This function initializes the OpenCL hardware resources (buffers, kernels, etc.) for hardware chaining tasks. The size of input/output buffers (decides the maximum no. of anchors that can be processed by any chaining task, and is limited by FPGA device's global memory) is specified as an argument. Call this function once before any `perform_core_chaining_on_fpga()` function call.

```c
void perform_core_chaining_on_fpga(int64_t n, int32_t max_dist_x, int32_t max_dist_y, int32_t bw, int32_t q_span, float avg_qspan_scaled, anchor_t* a, int32_t* f, int32_t* p, unsigned char* num_subparts, int64_t total_subparts, int32_t kernel_id);
```
This is the key function in *chain-fpga* that takes in the anchor data and other parameters corresponding to the chaining task and processes it (transfer the inputs to FPGA, perform chaining on FPGA, transfer the output to host) on the FPGA-based hardware accelerator. The example in `examples/hardware/` can be used as a guide to know how the arguments to this function should be set properly.

```c
void cleanup();
```
This function cleans up the OpenCL hardware resources allocated by `hardware_init()`. This should be called only once in your program after all the `perform_core_chaining_on_fpga()` calls are completed.

## Acknowledgment

The chaining algorithm used in *chain-fpga* was inspired by the core chaining computation of `mm_chain_dp` function in [Minimap2](https://github.com/lh3/minimap2).
