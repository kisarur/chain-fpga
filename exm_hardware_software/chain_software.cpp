#include <anchor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chain_software.h"
#include "common.h"

static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)};

static inline int ilog2_32(uint32_t v) {
    uint32_t t, tt;
    if ((tt = v >> 16)) return (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
    return (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
}

void perform_core_chaining_on_cpu(int64_t n, int32_t max_dist_x, int32_t max_dist_y, int32_t bw, float avg_qspan_scaled, int max_iter,
                                  anchor_t* a, int32_t* f, int32_t* p) {
    int32_t* t = (int32_t*)malloc(n * sizeof(int32_t));

    int64_t st = 0;
    for (int64_t i = 0; i < n; ++i) {
        uint64_t ri = a[i].x;
        int64_t max_j = -1;
        int32_t qi = (int32_t)a[i].y, q_span = a[i].y >> 32 & 0xff;  // NB: only 8 bits of span is used!!!
        int32_t max_f = q_span, n_skip = 0, min_d;
        int32_t sidi = (a[i].y & MM_SEED_SEG_MASK) >> MM_SEED_SEG_SHIFT;
        while (st < i && ri > a[st].x + max_dist_x) ++st;
        if (i - st > max_iter) st = i - max_iter;
        for (int64_t j = i - 1; j >= st; --j) {
            int64_t dr = ri - a[j].x;
            int32_t dq = qi - (int32_t)a[j].y, dd, sc, log_dd, gap_cost;
            int32_t sidj = (a[j].y & MM_SEED_SEG_MASK) >> MM_SEED_SEG_SHIFT;
            if ((sidi == sidj && dr == 0) || dq <= 0) continue;  // don't skip if an anchor is used by multiple segments; see below
            if ((sidi == sidj && dq > max_dist_y) || dq > max_dist_x) continue;
            dd = dr > dq ? dr - dq : dq - dr;
            if (sidi == sidj && dd > bw) continue;
            if (n_segs > 1 && !is_cdna && sidi == sidj && dr > max_dist_y) continue;
            min_d = dq < dr ? dq : dr;
            sc = min_d > q_span ? q_span : dq < dr ? dq
                                                   : dr;
            log_dd = dd ? ilog2_32(dd) : 0;
            gap_cost = 0;
            if (is_cdna || sidi != sidj) {
                int c_log, c_lin;
                c_lin = (int)(dd * avg_qspan_scaled);
                c_log = log_dd;
                if (sidi != sidj && dr == 0)
                    ++sc;  // possibly due to overlapping paired ends; give a minor bonus
                else if (dr > dq || sidi != sidj)
                    gap_cost = c_lin < c_log ? c_lin : c_log;
                else
                    gap_cost = c_lin + (c_log >> 1);
            } else
                gap_cost = (int)(dd * avg_qspan_scaled) + (log_dd >> 1);
            sc -= (int)((double)gap_cost * gap_scale + .499);
            sc += f[j];
#if !defined(ENABLE_MAX_SKIP_ON_SW) || defined(VERIFY_OUTPUT)
            if (sc > max_f) {
                max_f = sc, max_j = j;
            }
#else
            if (sc > max_f) {
                max_f = sc, max_j = j;
                if (n_skip > 0) --n_skip;
            } else if (t[j] == i) {
                if (++n_skip > max_skip)
                    break;
            }
            if (p[j] >= 0) t[p[j]] = i;
#endif
        }
        f[i] = max_f, p[i] = max_j;
    }

    free(t);
}