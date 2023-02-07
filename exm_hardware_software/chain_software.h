// Comment the following macro to disable max_skip heuristic for chaining on software
// (disabling this will improve accuracy and it's always disabled on FPGA hardware)
#define ENABLE_MAX_SKIP_ON_SW  
static int max_skip = 25;

// The following values are fixed on FPGA hardware so keep them constant 
// (these are the default values used in human reference-based mapping)
static int n_segs = 1;
static int is_cdna = 0;
static float gap_scale = 1.0;

#define MM_SEED_SEG_SHIFT 48
#define MM_SEED_SEG_MASK (0xffULL << (MM_SEED_SEG_SHIFT))

void perform_core_chaining_on_cpu(int64_t, int32_t, int32_t, int32_t, float, int, anchor_t *, int32_t *, int32_t *);