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

#include "include/ft_runner.h"

int ft_report_write_json(const ft_runner_context_t* ctx)
{
    if (ctx == NULL || ctx->output_path == NULL)
        return -1;

    FILE* fp = fopen(ctx->output_path, "w");
    if (fp == NULL)
        return -1;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"schema_version\": \"1.0.0\",\n");
    fprintf(fp, "  \"run_id\": \"%s\",\n", ctx->run_id);
    fprintf(fp, "  \"dut_id\": \"%s\",\n", ctx->dut_id);
    fprintf(fp, "  \"station_id\": \"%s\",\n", ctx->station_id);
    fprintf(fp, "  \"sdk_version\": \"1.0.0\",\n");
    fprintf(fp, "  \"start_time\": \"1970-01-01T00:00:00Z\",\n");
    fprintf(fp, "  \"end_time\": \"1970-01-01T00:00:01Z\",\n");
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"total\": %d,\n", ctx->summary.total);
    fprintf(fp, "    \"pass\": %d,\n", ctx->summary.pass);
    fprintf(fp, "    \"fail\": %d,\n", ctx->summary.fail);
    fprintf(fp, "    \"blocked\": %d,\n", ctx->summary.blocked);
    fprintf(fp, "    \"overall_status\": \"%s\"\n", ft_status_to_str(ctx->summary.overall));
    fprintf(fp, "  },\n");

    fprintf(fp, "  \"cases\": [\n");
    for (size_t i = 0; i < ctx->case_count; i++) {
        const ft_case_result_t* c = &ctx->cases[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"case_id\": \"%s\",\n", c->case_id);
        fprintf(fp, "      \"case_name\": \"%s\",\n", c->case_name);
        fprintf(fp, "      \"status\": \"%s\",\n", ft_status_to_str(c->status));
        fprintf(fp, "      \"bt_status\": %d,\n", c->bt_status);
        fprintf(fp, "      \"duration_ms\": %d,\n", c->duration_ms);
        fprintf(fp, "      \"metrics\": {\n");
        fprintf(fp, "        \"metric_a\": %d,\n", c->metric_a);
        fprintf(fp, "        \"metric_b\": %d\n", c->metric_b);
        fprintf(fp, "      },\n");
        fprintf(fp, "      \"verdict_reason\": \"%s\",\n", c->verdict_reason);
        fprintf(fp, "      \"trace_id\": \"%s\"\n", c->trace_id);
        fprintf(fp, i + 1 == ctx->case_count ? "    }\n" : "    },\n");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}
