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

#include <string.h>

#include "include/ft_runner.h"

const char* ft_status_to_str(ft_status_t status)
{
    switch (status) {
    case FT_STATUS_PASS:
        return "PASS";
    case FT_STATUS_FAIL:
        return "FAIL";
    case FT_STATUS_BLOCKED:
        return "BLOCKED";
    default:
        return "BLOCKED";
    }
}

static void update_summary(ft_run_summary_t* summary, ft_status_t status)
{
    if (summary == NULL)
        return;

    switch (status) {
    case FT_STATUS_PASS:
        summary->pass++;
        break;
    case FT_STATUS_FAIL:
        summary->fail++;
        break;
    case FT_STATUS_BLOCKED:
    default:
        summary->blocked++;
        break;
    }
}

int ft_runner_execute(ft_runner_context_t* ctx)
{
    if (ctx == NULL)
        return -1;

    memset(&ctx->summary, 0, sizeof(ctx->summary));
    ctx->case_count = 0;

    if (ft_case_ble_scan(&ctx->cases[ctx->case_count++]) < 0)
        return -1;
    if (ft_case_ble_conn(&ctx->cases[ctx->case_count++]) < 0)
        return -1;
    if (ft_case_gatt_loop(&ctx->cases[ctx->case_count++]) < 0)
        return -1;

    ctx->summary.total = (int)ctx->case_count;
    for (size_t i = 0; i < ctx->case_count; i++)
        update_summary(&ctx->summary, ctx->cases[i].status);

    if (ctx->summary.fail > 0)
        ctx->summary.overall = FT_STATUS_FAIL;
    else if (ctx->summary.blocked > 0)
        ctx->summary.overall = FT_STATUS_BLOCKED;
    else
        ctx->summary.overall = FT_STATUS_PASS;

    return 0;
}
