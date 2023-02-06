#define ENABLE_MAX_SKIP_ON_SW // enables max_skip heuristic for chaining on software

#define MM_SEED_SEG_SHIFT  48
#define MM_SEED_SEG_MASK   (0xffULL<<(MM_SEED_SEG_SHIFT))

static int n_segs = 1;
static int is_cdna = 0;
static int max_skip = 25;
static float gap_scale = 1.0;

void perform_core_chaining_on_cpu(int64_t, int32_t, int32_t, int32_t, float, int, anchor_t *, int32_t *, int32_t *);