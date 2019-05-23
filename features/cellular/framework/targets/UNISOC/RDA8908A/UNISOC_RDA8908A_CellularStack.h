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

#ifndef UNISOC_RDA8908A_CELLULARSTACK_H_
#define UNISOC_RDA8908A_CELLULARSTACK_H_

#include "AT_CellularStack.h"

#define RDA8908A_SOCKET_MAX 6
#define RDA8908A_CREATE_SOCKET_TIMEOUT 75000 //75 seconds
#define RDA8908A_SENT_BYTE_MAX 1460
#define RDA8908A_RECV_BYTE_MAX 1000

namespace mbed {

class UNISOC_RDA8908A_CellularStack : public AT_CellularStack {
public:
    UNISOC_RDA8908A_CellularStack(ATHandler &atHandler, int cid, nsapi_ip_stack_t stack_type);
    virtual ~UNISOC_RDA8908A_CellularStack();

protected: // NetworkStack

    virtual nsapi_error_t socket_listen(nsapi_socket_t handle, int backlog);

    virtual nsapi_error_t socket_accept(nsapi_socket_t server,
                                        nsapi_socket_t *handle, SocketAddress *address = 0);

protected: // AT_CellularStack
    virtual nsapi_error_t socket_stack_init();

    virtual int get_max_socket_count();

    virtual void handle_open_socket_response(int &modem_connect_id, int &err);

    virtual bool is_protocol_supported(nsapi_protocol_t protocol);

    virtual nsapi_error_t socket_close_impl(int sock_id);

    virtual nsapi_error_t create_socket_impl(CellularSocket *socket);

    virtual nsapi_size_or_error_t socket_sendto_impl(CellularSocket *socket, const SocketAddress &address,
                                                     const void *data, nsapi_size_t size);

    virtual nsapi_size_or_error_t socket_recvfrom_impl(CellularSocket *socket, SocketAddress *address,
                                                       void *buffer, nsapi_size_t size);

private:
    // URC handlers
    void urc_rxget();
    void urc_cscon();
};
} // namespace mbed
#endif /* UNISOC_RDA8908A_CELLULARSTACK_H_ */
