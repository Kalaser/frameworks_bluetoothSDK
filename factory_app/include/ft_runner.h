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

#ifndef __FACTORY_APP_FT_RUNNER_H__
#define __FACTORY_APP_FT_RUNNER_H__

#include <stdbool.h>
#include <stddef.h>

#define FT_MAX_CASES 16
#define FT_MAX_STR 64

typedef enum {
    FT_STATUS_PASS = 0,
    FT_STATUS_FAIL,
    FT_STATUS_BLOCKED,
} ft_status_t;

typedef struct {
    char case_id[FT_MAX_STR];
    char case_name[FT_MAX_STR];
    ft_status_t status;
    int bt_status;
    int duration_ms;
    int metric_a;
    int metric_b;
    char verdict_reason[FT_MAX_STR];
    char trace_id[FT_MAX_STR];
} ft_case_result_t;

typedef struct {
    int total;
    int pass;
    int fail;
    int blocked;
    ft_status_t overall;
} ft_run_summary_t;

typedef struct {
    const char* run_id;
    const char* dut_id;
    const char* station_id;
    const char* output_path;
    ft_case_result_t cases[FT_MAX_CASES];
    size_t case_count;
    ft_run_summary_t summary;
} ft_runner_context_t;

int ft_runner_execute(ft_runner_context_t* ctx);
int ft_report_write_json(const ft_runner_context_t* ctx);
const char* ft_status_to_str(ft_status_t status);

int ft_case_ble_scan(ft_case_result_t* result);
int ft_case_ble_conn(ft_case_result_t* result);
int ft_case_gatt_loop(ft_case_result_t* result);

#endif
