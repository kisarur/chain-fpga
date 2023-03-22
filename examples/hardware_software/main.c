#include <chain_fpga.h>
#include "common.h"
#include "chain_software.h"

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
	// this also finds the predicted hardware/software run-times of the chaining task
    long total_trip_count = 0;
    int64_t st = 0;
    int64_t total_subparts = 0;
    unsigned char *num_subparts = (unsigned char *)malloc((n + EXTRA_ELEMS) * sizeof(unsigned char));

    for (int64_t i = 0; i < n; i++) {
        while (st < i && a[i].x > a[st].x + max_dist_x) ++st;
        int inner_loop_trip_count = i - st;
        if (inner_loop_trip_count > FPGA_MAX_TRIPCOUNT) {
            inner_loop_trip_count = FPGA_MAX_TRIPCOUNT;
        }
        total_trip_count += inner_loop_trip_count;

        int subparts = inner_loop_trip_count / TRIPCOUNT_PER_SUBPART;
        if (inner_loop_trip_count == 0 || inner_loop_trip_count % TRIPCOUNT_PER_SUBPART > 0) subparts++;
        num_subparts[i] = (unsigned char)subparts;
        total_subparts += subparts;
    }

    float hw_time_pred = K1_HW * n + K2_HW * total_subparts + C_HW;
    float sw_time_pred = K_SW * total_trip_count + C_SW;

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

#ifndef VERIFY_OUTPUT

    if (hw_time_pred < sw_time_pred) {
        // perform core chaining computation on FPGA hardware kernel 0
        perform_core_chaining_on_fpga(n, max_dist_x, max_dist_y, bw, q_span_hw, avg_qspan_scaled, a, f, p, num_subparts, total_subparts, 0);
	} else {
		double start = realtime();
        // perform core chaining computation on CPU as software
        perform_core_chaining_on_cpu(n, max_dist_x, max_dist_y, bw, avg_qspan_scaled, 5000, a, f, p);
    }

#else

	// perform chaining on both CPU (reference) and FPGA and cross-check the outputs
	// note: for CPU chaining reference, the maximum trip count of the inner loop is set to FPGA_MAX_TRIPCOUNT (maximum trip count supported by hardware)
    int32_t *f_ref = (int32_t *)malloc((n + EXTRA_ELEMS) * sizeof(int32_t));
    int32_t *p_ref = (int32_t *)malloc((n + EXTRA_ELEMS) * sizeof(int32_t));

    perform_core_chaining_on_cpu(n, max_dist_x, max_dist_y, bw, avg_qspan_scaled, FPGA_MAX_TRIPCOUNT, a, f_ref, p_ref);
    perform_core_chaining_on_fpga(n, max_dist_x, max_dist_y, bw, q_span_hw, avg_qspan_scaled, a, f, p, num_subparts, total_subparts, 0);

    int mismatched = 0;
    for (int64_t i = 0; i < n; i++) {
        if (f[i] != f_ref[i] || p[i] != p_ref[i]) {
            // fprintf(stderr, "n = %ld, total_subparts = %d, i = %d | f = %d, f_hw = %d | p = %d, p_hw = %d | %d\n", n, total_subparts, i, f[i], f_hw[i], p[i], p_hw[i], num_subparts[i]);
            mismatched++;
        }
    }
    if (mismatched > 0) {
        fprintf(stderr, "[Warning] Hardware and software outputs mismatched (mismatch count : %d/%ld)\n", mismatched, n);
    } else {
        fprintf(stderr, "[Info] Hardware and software outputs matched.\n");
	}

    free(f_ref);
    free(p_ref);

#endif

    // free memory allocated
    free(num_subparts);
    free(a);
    free(f);
    free(p);

    // free the OpenCL FPGA resources allocated
    cleanup();

    return 0;
}

double realtime(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + tp.tv_usec * 1e-6;
}