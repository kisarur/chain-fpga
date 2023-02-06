# chaining_fpga_lib
Library for FPGA-based chaining step (genomics) accelerator 


### Perform chaining on FPGA-based hardware accelerator
```c
void perform_core_chaining_on_fpga(int64_t n, int max_dist_x, int32_t max_dist_y, int32_t bw, int32_t q_span, float avg_qspan_scaled, anchor_t * a, int32_t* f, int32_t* p, unsigned char * num_subparts, int64_t total_subparts, int kernel_id);
```
This function takes in the parameters and the anchor data corresponding to the chaining task and processes it (transfer the inputs to FPGA, perform chaining on FPGA, transfer the output to host) on the FPGA-based hardware accelerator. 
