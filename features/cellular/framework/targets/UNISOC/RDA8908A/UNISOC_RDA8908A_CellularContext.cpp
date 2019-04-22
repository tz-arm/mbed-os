/*
 * Copyright (c) 2018, Arm Limited and affiliates.
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
#include "UNISOC_RDA8908A_CellularContext.h"
#include "UNISOC_RDA8908A_CellularStack.h"

namespace mbed {

UNISOC_RDA8908A_CellularContext::UNISOC_RDA8908A_CellularContext(ATHandler &at, CellularDevice *device, const char *apn) :
    AT_CellularContext(at, device, apn)
{
}

UNISOC_RDA8908A_CellularContext::~UNISOC_RDA8908A_CellularContext()
{
}

NetworkStack *UNISOC_RDA8908A_CellularContext::get_stack()
{
    if (!_stack) {
        _stack = new UNISOC_RDA8908A_CellularStack(_at, _cid, _ip_stack_type);
    }
    return _stack;
}

bool UNISOC_RDA8908A_CellularContext::stack_type_supported(nsapi_ip_stack_t stack_type)
{
    return stack_type == IPV4_STACK ? true : false;
}

nsapi_error_t UNISOC_RDA8908A_CellularContext::do_user_authentication()
{

    _at.cmd_start("AT+CIPCSGP=");
    _at.write_int(1); /*GPRS MODE = 1, CSD MODE = 0*/
    _at.write_string(_apn);
    if (_pwd && _uname) {
        _at.write_string(_uname);
        _at.write_string(_pwd);
    }
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();
    if (_at.get_last_error() != NSAPI_ERROR_OK) {
        return NSAPI_ERROR_AUTH_FAILURE;
    }

    return NSAPI_ERROR_OK;
}


} /* namespace mbed */
