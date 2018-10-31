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

#include "QUECTEL/M26/QUECTEL_M26_CellularNetwork.h"
#include "QUECTEL/M26/QUECTEL_M26_CellularStack.h"

using namespace mbed;

QUECTEL_M26_CellularNetwork::QUECTEL_M26_CellularNetwork(ATHandler &atHandler) : AT_CellularNetwork(atHandler)
{
}

QUECTEL_M26_CellularNetwork::~QUECTEL_M26_CellularNetwork()
{
}

NetworkStack *QUECTEL_M26_CellularNetwork::get_stack()
{
    if (!_stack) {
        _stack = new QUECTEL_M26_CellularStack(_at, _cid, _ip_stack_type);
    }
    return _stack;
}

bool QUECTEL_M26_CellularNetwork::get_modem_stack_type(nsapi_ip_stack_t requested_stack)
{
    return requested_stack == IPV4_STACK ? true : false;
}

AT_CellularNetwork::RegistrationMode QUECTEL_M26_CellularNetwork::has_registration(RegistrationType reg_type)
{
    return (reg_type == C_REG || reg_type == C_GREG) ? RegistrationModeLAC : RegistrationModeDisable;
}

nsapi_error_t QUECTEL_M26_CellularNetwork::set_access_technology_impl(RadioAccessTechnology opRat)
{
    _op_act = RAT_UNKNOWN;
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_error_t QUECTEL_M26_CellularNetwork::do_user_authentication()
{
    
        _at.cmd_start("AT+QICSGP=");
        _at.write_int(1); // mode: 0-CSD, 1-GPRS
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
