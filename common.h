#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct { 
    uint64_t x; 
    uint64_t y; 
} anchor_t;

// Parameters used in HW/SW split
#define K1_HW 8.429346604594088e-06
#define K2_HW 4.219985490696127e-05
#define C_HW 0.6198209995910657
#define K_SW 5.237303730088228e-06
#define C_SW -0.9308447100947506

#define VERIFY_OUTPUT            // to run both on software and hardware to cross-check the outputs
#define PROCESS_ON_SW_IF_HW_BUSY // controls whether to process chaining tasks (chosen for hardware) on software if it's more suitable to do so

double realtime(void);
