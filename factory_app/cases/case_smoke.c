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

#include "../include/ft_runner.h"

static int fill_case(ft_case_result_t* result,
    const char* case_id,
    const char* case_name,
    int duration,
    int metric_a,
    int metric_b)
{
    if (result == NULL)
        return -1;

    memset(result, 0, sizeof(*result));
    snprintf(result->case_id, sizeof(result->case_id), "%s", case_id);
    snprintf(result->case_name, sizeof(result->case_name), "%s", case_name);
    result->status = FT_STATUS_PASS;
    result->bt_status = 0;
    result->duration_ms = duration;
    result->metric_a = metric_a;
    result->metric_b = metric_b;
    snprintf(result->verdict_reason, sizeof(result->verdict_reason), "ok");
    snprintf(result->trace_id, sizeof(result->trace_id), "TRACE-%s", case_id);

    return 0;
}

int ft_case_ble_scan(ft_case_result_t* result)
{
    return fill_case(result, "FT_BLE_SCAN_001", "ble_scan_count_and_rssi", 1000, 6, -60);
}

int ft_case_ble_conn(ft_case_result_t* result)
{
    return fill_case(result, "FT_CONN_001", "ble_connect_latency", 2100, 2100, 0);
}

int ft_case_gatt_loop(ft_case_result_t* result)
{
    return fill_case(result, "FT_GATT_LOOP_001", "gatt_read_write_loop", 700, 82, 100);
}
