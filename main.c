#include "common.h"
#include "chain_hardware.h"
#include "chain_software.h"

int main(int argc, char * argv[]) {

	if (argc != 2) {
		printf("Usage: %s <anchor_data_file>\n", argv[0]);
		return 1;
  	}
	char * filename = argv[1];

    // initialize OpenCL FPGA communication resources (input/output buffers, kernels, command queues, etc)
	if (!hardware_init(BUFFER_N)) {
        return 1;
    }

	// read chaining task's data from file
	int64_t n;
	int32_t max_dist_x, max_dist_y, bw; 

	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("[Error] Could not open the anchor data file to read!\n");
		return 1;
	}

	fscanf(fp, "%ld%d%d%d", &n, &max_dist_x, &max_dist_y, &bw); 

	anchor_t * a = (anchor_t*)malloc(n * sizeof(anchor_t));

	for (int64_t i = 0; i < n; i++) {
		fscanf(fp, "%lu%lu", &a[i].x, &a[i].y);
	}

	fclose(fp);

	// perform core chaining computation on HW/SW
	int32_t * f = (int32_t*)malloc(n * sizeof(int32_t));
	int32_t * p = (int32_t*)malloc(n * sizeof(int32_t));
    
	uint64_t sum_qspan = 0;
	float avg_qspan_scaled;
    for (int64_t i = 0; i < n; ++i) sum_qspan += a[i].y>>32&0xff;
	avg_qspan_scaled = .01 * (float)sum_qspan / n;

	/*--------- HW/SW time prediction Start ------------*/
	
	long total_trip_count = 0;
	int64_t st = 0;
	int64_t total_subparts = 0;
	unsigned char * num_subparts = (unsigned char*)malloc((n + EXTRA_ELEMS) * sizeof(unsigned char));

	for (int64_t i = 0; i < n; i++) {
		// determine and store the inner loop's trip count (max is INNER_LOOP_TRIP_COUNT_MAX)
		while (st < i && a[i].x > a[st].x + max_dist_x) ++st;
		int inner_loop_trip_count = i - st;
		if (inner_loop_trip_count > MAX_TRIPCOUNT) { 
			inner_loop_trip_count = MAX_TRIPCOUNT;
		}
		total_trip_count += inner_loop_trip_count;

		int subparts = inner_loop_trip_count / TRIPCOUNT_PER_SUBPART;
		if (inner_loop_trip_count == 0 || inner_loop_trip_count % TRIPCOUNT_PER_SUBPART > 0) subparts++;
		num_subparts[i] = (unsigned char)subparts;
		total_subparts += subparts;
	}

	float hw_time_pred = K1_HW * n + K2_HW * total_subparts + C_HW;
	float sw_time_pred = K_SW * total_trip_count + C_SW;
	
	/*--------- HW/SW time prediction End ------------*/

	int q_span_hw = n > 0 ? a[0].y>>32&0xff : 0;

#ifndef VERIFY_OUTPUT

	if (hw_time_pred < sw_time_pred) { // execute on HW

		int fpga_chain = perform_core_chaining_on_fpga(n, max_dist_x, max_dist_y, bw, q_span_hw, avg_qspan_scaled, a, f, p, num_subparts, total_subparts, 0, hw_time_pred, sw_time_pred);

#ifdef PROCESS_ON_SW_IF_HW_BUSY
		if (fpga_chain != 0) { // execute on SW
			perform_core_chaining_on_cpu(n, max_dist_x, max_dist_y, bw, avg_qspan_scaled, 5000, a, f, p);
		}
#endif

	} else { // execute on SW
		perform_core_chaining_on_cpu(n, max_dist_x, max_dist_y, bw, avg_qspan_scaled, 5000, a, f, p);
	}

#else

	int32_t * f_hw = (int32_t*)malloc((n + EXTRA_ELEMS) * sizeof(int32_t));
	int32_t * p_hw = (int32_t*)malloc((n + EXTRA_ELEMS) * sizeof(int32_t));

	perform_core_chaining_on_cpu(n, max_dist_x, max_dist_y, bw, avg_qspan_scaled, MAX_TRIPCOUNT, a, f, p);
	perform_core_chaining_on_fpga(n, max_dist_x, max_dist_y, bw, q_span_hw, avg_qspan_scaled, a, f_hw, p_hw, num_subparts, total_subparts, 0, 0, 0);

	int mismatched = 0;
	for (int64_t i = 0; i < n; i++) {
		if (f[i] != f_hw[i] || p[i] != p_hw[i]) {
			// fprintf(stderr, "n = %ld, total_subparts = %d, i = %d | f = %d, f_hw = %d | p = %d, p_hw = %d | %d\n", n, total_subparts, i, f[i], f_hw[i], p[i], p_hw[i], num_subparts[i]);
			mismatched++;
		}
	}
	if (mismatched > 0) {
		fprintf(stderr, "mismatched = %d/%ld\n", mismatched, n);
	}

	free(f_hw);
	free(p_hw);
	
#endif

	free(num_subparts);

	free(a);
	free(f);
	free(p);

	// free the OpenCL FPGA resources allocated
    cleanup();

    return 0;
}