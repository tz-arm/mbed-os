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

#include "UNISOC_RDA8908A_CellularNetwork.h"

using namespace mbed;

UNISOC_RDA8908A_CellularNetwork::UNISOC_RDA8908A_CellularNetwork(ATHandler &atHandler) : AT_CellularNetwork(atHandler)
{
    _op_act = RAT_NB1;
}

UNISOC_RDA8908A_CellularNetwork::~UNISOC_RDA8908A_CellularNetwork()
{
}

AT_CellularNetwork::RegistrationMode UNISOC_RDA8908A_CellularNetwork::has_registration(RegistrationType reg_tech)
{
    return (reg_tech == C_EREG) ? RegistrationModeLAC : RegistrationModeDisable;
}

nsapi_error_t UNISOC_RDA8908A_CellularNetwork::set_access_technology_impl(RadioAccessTechnology opRat)
{
    if (opRat != RAT_NB1) {
        // only rat that is supported by this modem
        _op_act = RAT_NB1;
        return NSAPI_ERROR_UNSUPPORTED;
    }

    return NSAPI_ERROR_OK;
}
