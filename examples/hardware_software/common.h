// Parameters used for hardware/software run-time prediction
#define K1_HW 8.429346604594088e-06
#define K2_HW 4.219985490696127e-05
#define C_HW 0.6198209995910657
#define K_SW 5.237303730088228e-06
#define C_SW -0.9308447100947506

// Uncomment the following macro to run both on software and FPGA hardware and cross-check the outputs
// (enabling this macro will disable max_skip heuristic on software)
#define VERIFY_OUTPUT

double realtime(void);
