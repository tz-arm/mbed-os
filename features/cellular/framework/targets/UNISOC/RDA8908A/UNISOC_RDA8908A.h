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

#ifndef UNISOC_RDA8908A_H_
#define UNISOC_RDA8908A_H_

#include "AT_CellularDevice.h"

namespace mbed {

class UNISOC_RDA8908A : public AT_CellularDevice {
public:
    UNISOC_RDA8908A(FileHandle *fh);

protected: // AT_CellularDevice
    virtual AT_CellularNetwork *open_network_impl(ATHandler &at);
    virtual AT_CellularContext *create_context_impl(ATHandler &at, const char *apn, bool cp_req = false, bool nonip_req = false);
public: // NetworkInterface
    void handle_urc(FileHandle *fh);
};
} // namespace mbed
#endif // UNISOC_RDA8908A_H_
