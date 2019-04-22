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
#include "UNISOC_RDA8908A_CellularPower.h"
#include "UNISOC_RDA8908A_CellularSIM.h"
#include "UNISOC_RDA8908A_CellularContext.h"
#include "UNISOC_RDA8908A.h"

#define CONNECT_DELIM         "\r\n"
#define CONNECT_BUFFER_SIZE   (1280 + 80 + 80) // AT response + sscanf format
#define CONNECT_TIMEOUT       8000

#define MAX_STARTUP_TRIALS 5
#define MAX_RESET_TRIALS 5

using namespace events;
using namespace mbed;

static const AT_CellularBase::SupportedFeature unsupported_features[] =  {
    AT_CellularBase::AT_CGAUTH, 
    AT_CellularBase::SUPPORTED_FEATURE_END_MARK
};

UNISOC_RDA8908A::UNISOC_RDA8908A(FileHandle *fh) : AT_CellularDevice(fh)
{
    AT_CellularBase::set_unsupported_features(unsupported_features);
}

UNISOC_RDA8908A::~UNISOC_RDA8908A()
{
}

AT_CellularNetwork *UNISOC_RDA8908A::open_network_impl(ATHandler &at)
{
    return new UNISOC_RDA8908A_CellularNetwork(at);
}

AT_CellularPower *UNISOC_RDA8908A::open_power_impl(ATHandler &at)
{
    return new UNISOC_RDA8908A_CellularPower(at);
}

AT_CellularSIM *UNISOC_RDA8908A::open_sim_impl(ATHandler &at)
{
    return new UNISOC_RDA8908A_CellularSIM(at);
}

AT_CellularContext *UNISOC_RDA8908A::create_context_impl(ATHandler &at, const char *apn)
{
    return new UNISOC_RDA8908A_CellularContext(at, this, apn);
}
