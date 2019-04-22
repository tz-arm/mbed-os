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

#include "UNISOC_RDA8908A_CellularStack.h"
#include "CellularUtil.h"
#include "CellularLog.h"

using namespace mbed;
using namespace mbed_cellular_util;

UNISOC_RDA8908A_CellularStack::UNISOC_RDA8908A_CellularStack(ATHandler &atHandler, int cid, nsapi_ip_stack_t stack_type) : AT_CellularStack(atHandler, cid, stack_type)
{
    _at.set_urc_handler("+RECEIVE:", mbed::Callback<void()>(this, &UNISOC_RDA8908A_CellularStack::urc_nsonmi));
}

UNISOC_RDA8908A_CellularStack::~UNISOC_RDA8908A_CellularStack()
{
}

// OK
nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_listen(nsapi_socket_t handle, int backlog)
{
    //TBD
    return NSAPI_ERROR_UNSUPPORTED;
}
// OK
nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    //TBD
    return NSAPI_ERROR_UNSUPPORTED;
}


void UNISOC_RDA8908A_CellularStack::urc_nsonmi()
{
    _at.lock();
    int sock_id = _at.read_int();
    (void) _at.skip_param(); /*<len>*/
    (void) _at.skip_param(); /*<tlen>*/
    (void) _at.skip_param(); /*<tlen>*/
    _at.unlock();

    for (int i = 0; i < get_max_socket_count(); i++) {
        CellularSocket *sock = _socket[i];
        if (sock && sock->id == sock_id) {
            if (sock->_cb) {
                sock->_cb(sock->_data);
            }
            break;
        }
    }
}

nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_stack_init()
{
    int set_mux = 0;
    int set_showip = 0;
    int set_rxmode = 0;
    //tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START ", __FUNCTION__, __LINE__);

    _at.lock();
    _at.cmd_start("AT+CIPMUX?");
    _at.cmd_stop();
    _at.resp_start("+CIPMUX:");
    if (_at.info_resp()) {
        set_mux = _at.read_int();
    }
    _at.resp_stop();
    if(!set_mux){
        _at.cmd_start("AT+CIPMUX=");
        _at.write_int(1); /* 0-Disable, 1-Enable */
        _at.cmd_stop_read_resp();
    }

#if 1
    /* AT+CSTT Start Task And Set APN, User ID, Password */
    _at.cmd_start("AT+CSTT=");
    _at.write_string("snbiot");
    _at.write_string("");
    _at.write_string("");
    _at.cmd_stop_read_resp();

    /*AT+CIICR Bring Up Wireless Connection With GPRS*/
    _at.cmd_start("AT+CIICR");
    _at.cmd_stop_read_resp();
#endif

    /*AT+CIFSR Get Local IP Address*/
    _at.cmd_start("AT+CIFSR");
    _at.cmd_stop_read_resp();

    //////////////////////////////////////
    _at.cmd_start("AT+CIPSRIP?");
    _at.cmd_stop();
    _at.resp_start("+CIPSRIP:");
    if (_at.info_resp()) {
        set_showip = _at.read_int();
    }
    _at.resp_stop();
    if(!set_showip){
        _at.cmd_start("AT+CIPSRIP=");
        _at.write_int(1); /* 0-Disable, 1-Enable */
        _at.cmd_stop_read_resp();
    }

    //////////////////////////////////////
    _at.cmd_start("AT+CIPRXGET?");
    _at.cmd_stop();
    _at.resp_start("+CIPRXGET:");
    if (_at.info_resp()) {
        set_rxmode = _at.read_int();
    }
    _at.resp_stop();
    if(!set_rxmode){
        _at.cmd_start("AT+CIPRXGET=");
        _at.write_int(1); 
        _at.cmd_stop_read_resp();
    }



/*
    _at.cmd_start("AT+CIPQSEND?");
    _at.cmd_stop();
    _at.resp_start("+CIPQSEND:");
    if (_at.info_resp()) {
        tmp = _at.read_int();
    }
    _at.resp_stop();
    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: CIPQSEND = %d ", __FUNCTION__, __LINE__,tmp);
*/

    //tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END ", __FUNCTION__, __LINE__);
    return _at.unlock_return_error();
}

// OK
int UNISOC_RDA8908A_CellularStack::get_max_socket_count()
{
    return RDA8908A_SOCKET_MAX;
}

// OK
bool UNISOC_RDA8908A_CellularStack::is_protocol_supported(nsapi_protocol_t protocol)
{
    return (protocol == NSAPI_UDP || protocol == NSAPI_TCP);
}

// OK
nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_close_impl(int sock_id)
{
    _at.cmd_start("AT+CIPCLOSE=");
    _at.write_int(sock_id);
    _at.cmd_stop_read_resp();

    tr_info("Close socket: %d error: %d", sock_id, _at.get_last_error());

    return _at.get_last_error();
}

void UNISOC_RDA8908A_CellularStack::handle_open_socket_response(int &modem_connect_id, int &err)
{
    char status[15];
    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START", __FUNCTION__, __LINE__);

    _at.resp_start();
    if (_at.info_resp()) {
        tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END [%s, %d]", __FUNCTION__, __LINE__, status, err);
        modem_connect_id = _at.read_int();
        _at.read_string(status, sizeof(status));
        if (!strcmp(status, "CONNECT OK"))
            err = 0;
        else if(!strcmp(status, "ALREADY CONNECT"))
            err = 1;   
    }
    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END [%s, %d]", __FUNCTION__, __LINE__, status, err);
}

nsapi_error_t UNISOC_RDA8908A_CellularStack::create_socket_impl(CellularSocket *socket)
{
    int request_connect_id;
    int modem_connect_id;
    int err = -1;
    nsapi_error_t ret_val;

    if ((socket->created)||(socket->id>=RDA8908A_SOCKET_MAX)) {
        tr_warn("UNISOC_RDA8908A_CellularStack:%s:%u: create_socket_impl fail with parameter error:%d,%d,%d", __FUNCTION__, __LINE__,socket->connected,socket->created,socket->id);
        return NSAPI_ERROR_PARAMETER;
    }

    if (!socket->connected){
        tr_warn("UNISOC_RDA8908A_CellularStack:%s:%u: remote ip and port are not set.", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_OK;
    }

    request_connect_id = socket->id;
    modem_connect_id = request_connect_id;

    _at.cmd_start("AT+CIPSTART=");
    _at.write_int(request_connect_id);
    _at.write_string((socket->proto == NSAPI_TCP) ? "TCP" : "UDP");
    _at.write_string(socket->remoteAddress.get_ip_address());
    _at.write_int(socket->remoteAddress.get_port());
    _at.cmd_stop_read_resp();

    handle_open_socket_response(modem_connect_id, err);
    if(err==1){
        tr_warn("UNISOC_RDA8908A_CellularStack:%s:%u: socket %d already connected.", __FUNCTION__, __LINE__,request_connect_id);
        /* ALREADY CONNECT: Close the curent connection and reconnect again. */
        _at.cmd_start("AT+QICLOSE=");
        _at.write_int(modem_connect_id);
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();

        _at.cmd_start("AT+CIPSTART=");
        _at.write_int(request_connect_id);
        _at.write_string((socket->proto == NSAPI_TCP) ? "TCP" : "UDP");
        _at.write_string(socket->remoteAddress.get_ip_address());
        _at.write_int(socket->remoteAddress.get_port());
        _at.cmd_stop_read_resp();

        handle_open_socket_response(modem_connect_id, err);
    }

    if (modem_connect_id != request_connect_id) {
        _at.cmd_start("AT+CIPCLOSE=");
        _at.write_int(modem_connect_id);
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();
        tr_warn("UNISOC_RDA8908A_CellularStack:%s:%u: created connect id is not the requested one.", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_NO_CONNECTION;
    }

    ret_val = _at.get_last_error();
    if ((ret_val!= NSAPI_ERROR_OK) || (err==-1)) {
        tr_warn("UNISOC_RDA8908A_CellularStack:%s:%u: Create socket failed err = %d ret_val = %d", __FUNCTION__, __LINE__, err, ret_val);
        return NSAPI_ERROR_NO_CONNECTION;
    }

    socket->created = true;

    return NSAPI_ERROR_OK;
}

nsapi_size_or_error_t UNISOC_RDA8908A_CellularStack::socket_sendto_impl(CellularSocket *socket, const SocketAddress &address,
                                                                     const void *data, nsapi_size_t size)
{
    int sent_len = 0;

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u:[%d-%d]", __FUNCTION__, __LINE__,size);

    if ((!socket)||(!data)||(size == 0)) {
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_PARAMETER]", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_PARAMETER;
    }

    if ((socket->proto == NSAPI_UDP)&&(!socket->created)) {
        socket->remoteAddress = address;
        socket->connected = true;
        nsapi_error_t ret_val = create_socket_impl(socket);
        if ((ret_val != NSAPI_ERROR_OK) || (!socket->created)) {
            tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_NO_SOCKET]", __FUNCTION__, __LINE__);
            return NSAPI_ERROR_NO_SOCKET;
        }
    }

    _at.cmd_start("AT+CIPSEND=");
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();
    _at.resp_start(">");
    if (_at.info_resp()) {
        _at.write_bytes((uint8_t *)data, size);
    }
    _at.resp_stop();

    if (_at.get_last_error() != NSAPI_ERROR_OK) {
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_DEVICE_ERROR]", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return size;
}

nsapi_size_or_error_t UNISOC_RDA8908A_CellularStack::socket_recvfrom_impl(CellularSocket *socket, SocketAddress *address,
                                                                       void *buffer, nsapi_size_t size)
{
    nsapi_size_or_error_t recv_len = 0;
    int port;
    char ip_address[NSAPI_IP_SIZE];

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u:[%d-%d]", __FUNCTION__, __LINE__,size);

    if ((socket == NULL)||(buffer==NULL)||(size==0)) {
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_PARAMETER]", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_PARAMETER;
    }

    _at.cmd_start("AT+CIPRXGET=");
    _at.write_int(2);
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();

    _at.resp_start("+CIPRXGET:");
    if (_at.info_resp()) {
        _at.skip_param();
        _at.skip_param();
        _at.skip_param();
        recv_len = _at.read_int();
        if (recv_len > 0) {
            _at.read_string(ip_address, sizeof(ip_address));
            port = _at.read_int();
            _at.read_bytes((uint8_t *)buffer, recv_len);
        }
    }
    _at.resp_stop();

    if (!recv_len || (_at.get_last_error() != NSAPI_ERROR_OK)) {
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    if (address) {
        address->set_ip_address(ip_address);
        address->set_port(port);
    }

    return recv_len;
}