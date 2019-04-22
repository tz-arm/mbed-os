/*
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UNISOC_RDA8908A_CellularPower.h"

using namespace mbed;

UNISOC_RDA8908A_CellularPower::UNISOC_RDA8908A_CellularPower(ATHandler &atHandler) : AT_CellularPower(atHandler)
{

}

UNISOC_RDA8908A_CellularPower::~UNISOC_RDA8908A_CellularPower()
{

}

nsapi_error_t UNISOC_RDA8908A_CellularPower::set_power_level(int func_level, int do_reset)
{
    _at.lock();
    if(func_level)
    if((func_level>1)||(do_reset>1))
        return NSAPI_ERROR_UNSUPPORTED;
    _at.cmd_start("AT+CFUN=");
    _at.write_int(func_level);
    _at.write_int(do_reset);
    _at.cmd_stop_read_resp();
    return _at.unlock_return_error();
}

nsapi_error_t UNISOC_RDA8908A_CellularPower::opt_power_save_mode(int periodic_time, int active_time)
{
    //TBD
    return NSAPI_ERROR_UNSUPPORTED;
}
