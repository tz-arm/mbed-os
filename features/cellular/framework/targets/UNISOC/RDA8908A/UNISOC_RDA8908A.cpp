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
#include "UNISOC_RDA8908A_CellularContext.h"
#include "UNISOC_RDA8908A.h"

#define CONNECT_DELIM         "\r\n"
#define CONNECT_BUFFER_SIZE   (1280 + 80 + 80) // AT response + sscanf format
#define CONNECT_TIMEOUT       8000

#define MAX_STARTUP_TRIALS 5
#define MAX_RESET_TRIALS 5

using namespace events;
using namespace mbed;

static const intptr_t cellular_properties[AT_CellularBase::PROPERTY_MAX] = {
    AT_CellularNetwork::RegistrationModeLAC,    // C_EREG
    AT_CellularNetwork::RegistrationModeDisable,// C_GREG
    AT_CellularNetwork::RegistrationModeDisable,// C_REG
    1,  // AT_CGSN_WITH_TYPE
    1,  // AT_CGDATA
    0,  // AT_CGAUTH
    1,  // PROPERTY_IPV4_STACK
    0,  // PROPERTY_IPV6_STACK
    0,  // PROPERTY_IPV4V6_STACK
};

UNISOC_RDA8908A::UNISOC_RDA8908A(FileHandle *fh) : AT_CellularDevice(fh)
{
    AT_CellularBase::set_cellular_properties(cellular_properties);
}

AT_CellularNetwork *UNISOC_RDA8908A::open_network_impl(ATHandler &at)
{
    return new UNISOC_RDA8908A_CellularNetwork(at);
}

AT_CellularContext *UNISOC_RDA8908A::create_context_impl(ATHandler &at, const char *apn, bool cp_req, bool nonip_req)
{
    return new UNISOC_RDA8908A_CellularContext(at, this, apn, cp_req, nonip_req);
}


#if MBED_CONF_UNISOC_RDA8908A_PROVIDE_DEFAULT
#include "UARTSerial.h"
CellularDevice *CellularDevice::get_default_instance()
{
    static UARTSerial serial(MBED_CONF_UNISOC_RDA8908A_TX, MBED_CONF_UNISOC_RDA8908A_RX, MBED_CONF_UNISOC_RDA8908A_BAUDRATE);
#if defined (MBED_CONF_UNISOC_RDA8908A_RTS) && defined(MBED_CONF_UNISOC_RDA8908A_CTS)
    tr_debug("UNISOC_RDA8908A flow control: RTS %d CTS %d", MBED_CONF_UNISOC_RDA8908A_RTS, MBED_CONF_UNISOC_RDA8908A_CTS);
    serial.set_flow_control(SerialBase::RTSCTS, MBED_CONF_UNISOC_RDA8908A_RTS, MBED_CONF_UNISOC_RDA8908A_CTS);
#endif
    static UNISOC_RDA8908A device(&serial);
    return &device;
}
#endif
