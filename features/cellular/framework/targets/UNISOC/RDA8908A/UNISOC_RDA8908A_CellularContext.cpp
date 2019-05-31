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
#include "CellularUtil.h"
#include "CellularLog.h"
#include "UARTSerial.h"

namespace mbed {

UNISOC_RDA8908A_CellularContext::UNISOC_RDA8908A_CellularContext(ATHandler &at, CellularDevice *device, const char *apn, bool cp_req, bool nonip_req) :
    AT_CellularContext(at, device, apn, cp_req, nonip_req)
{
}

UNISOC_RDA8908A_CellularContext::~UNISOC_RDA8908A_CellularContext()
{
}

#if !NSAPI_PPP_AVAILABLE
NetworkStack *UNISOC_RDA8908A_CellularContext::get_stack()
{
    if (_pdp_type == NON_IP_PDP_TYPE || _cp_in_use) {
        tr_error("Requesting stack for NON-IP context! Should request control plane netif: get_cp_netif()");
        return NULL;
    }

    if (!_stack) {
        _stack = new UNISOC_RDA8908A_CellularStack(_at, _cid, (nsapi_ip_stack_t)_pdp_type);
    }

    return _stack;
}
#endif // #if !NSAPI_PPP_AVAILABLE

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
    _at.cmd_stop_read_resp();

    if (_at.get_last_error() != NSAPI_ERROR_OK) {
        return NSAPI_ERROR_AUTH_FAILURE;
    }

    return NSAPI_ERROR_OK;
}


} /* namespace mbed */
