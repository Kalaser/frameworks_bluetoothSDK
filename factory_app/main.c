/****************************************************************************
 *  Copyright (C) 2026 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "include/ft_runner.h"

static void usage(const char* prog)
{
    printf("Usage: %s [--run-id id] [--dut-id id] [--station id] [--output path]\n", prog);
}

int main(int argc, char* argv[])
{
    ft_runner_context_t ctx = {
        .run_id = "RUN-LOCAL-0001",
        .dut_id = "DUT-UNKNOWN",
        .station_id = "S00",
        .output_path = "./ft_result.json",
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--run-id") == 0 && i + 1 < argc) {
            ctx.run_id = argv[++i];
        } else if (strcmp(argv[i], "--dut-id") == 0 && i + 1 < argc) {
            ctx.dut_id = argv[++i];
        } else if (strcmp(argv[i], "--station") == 0 && i + 1 < argc) {
            ctx.station_id = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            ctx.output_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "unknown arg: %s\n", argv[i]);
            usage(argv[0]);
            return -1;
        }
    }

    if (ft_runner_execute(&ctx) < 0) {
        fprintf(stderr, "runner execute failed\n");
        return -1;
    }

    if (ft_report_write_json(&ctx) < 0) {
        fprintf(stderr, "write report failed\n");
        return -1;
    }

    printf("ft_runner done: output=%s, overall=%s\n",
        ctx.output_path, ft_status_to_str(ctx.summary.overall));
    return 0;
}
