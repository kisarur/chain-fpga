#include <chain_fpga.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <anchor_data_file>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];

    // initialize OpenCL FPGA resources (input/output buffers, kernels, command queues, etc)
    if (!hardware_init(BUFFER_N)) return 1;

    // read chaining task's data from file
    int64_t n;
    int32_t max_dist_x, max_dist_y, bw;
    anchor_t *a = (anchor_t *)malloc(n * sizeof(anchor_t));
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("[Error] Could not open the anchor data file to read!\n");
        return 1;
    }
    fscanf(fp, "%ld%d%d%d", &n, &max_dist_x, &max_dist_y, &bw);
    for (int64_t i = 0; i < n; i++) {
        fscanf(fp, "%lu%lu", &a[i].x, &a[i].y);
    }
    fclose(fp);

    // find number of subaprts for each anchor and the total number of subparts
    int64_t st = 0;
    int64_t total_subparts = 0;
    unsigned char *num_subparts = (unsigned char *)malloc((n + EXTRA_ELEMS) * sizeof(unsigned char));

    for (int64_t i = 0; i < n; i++) {
        while (st < i && a[i].x > a[st].x + max_dist_x)
            ++st;
        int inner_loop_trip_count = i - st;
        if (inner_loop_trip_count > FPGA_MAX_TRIPCOUNT) {
            inner_loop_trip_count = FPGA_MAX_TRIPCOUNT;
        }

        int subparts = inner_loop_trip_count / TRIPCOUNT_PER_SUBPART;
        if (inner_loop_trip_count == 0 || inner_loop_trip_count % TRIPCOUNT_PER_SUBPART > 0)
            subparts++;
        num_subparts[i] = (unsigned char)subparts;
        total_subparts += subparts;
    }

    // find scaled average seed length
    uint64_t sum_qspan = 0;
    float avg_qspan_scaled;
    for (int64_t i = 0; i < n; ++i)
        sum_qspan += a[i].y >> 32 & 0xff;
    avg_qspan_scaled = .01 * (float)sum_qspan / n;

    // get seed length that's sent to hardware (assumes to be constant for all anchors)
    int q_span_hw = n > 0 ? a[0].y >> 32 & 0xff : 0;

    // allocate memory to store chaining scores and best predecessors
    int32_t *f = (int32_t *)malloc(n * sizeof(int32_t));
    int32_t *p = (int32_t *)malloc(n * sizeof(int32_t));

    // perform core chaining computation on FPGA hardware kernel 0
    perform_core_chaining_on_fpga(n, max_dist_x, max_dist_y, bw, q_span_hw, avg_qspan_scaled, a, f, p, num_subparts, total_subparts, 0);

    // print out chaining scores and best predecessors
    for (int64_t i = 0; i < n; i++) {
        printf("%d\t%d\n", f[i], p[i]);
    }

    // free memory allocated
    free(num_subparts);
    free(a);
    free(f);
    free(p);

    // free the OpenCL FPGA resources allocated
    cleanup();

    return 0;
}