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
#include "CellularContext.h"
#include "CellularNetwork.h"
#include "CellularUtil.h"
#include "CellularLog.h"

using namespace mbed;
using namespace mbed_cellular_util;


UNISOC_RDA8908A_CellularStack::UNISOC_RDA8908A_CellularStack(ATHandler &atHandler, int cid, nsapi_ip_stack_t stack_type) : AT_CellularStack(atHandler, cid, stack_type)
{
    // Module firmware not stable for urc, to be updated.
    //_at.set_urc_handler("+RECEIVE", mbed::Callback<void()>(this, &UNISOC_RDA8908A_CellularStack::urc_rxget));
}

UNISOC_RDA8908A_CellularStack::~UNISOC_RDA8908A_CellularStack()
{
}

nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_listen(nsapi_socket_t handle, int backlog)
{
    //TBD
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    //TBD
    return NSAPI_ERROR_UNSUPPORTED;
}


void UNISOC_RDA8908A_CellularStack::urc_rxget()
{
    for (int i = 0; i < get_max_socket_count(); i++) {
        CellularSocket *sock = _socket[i];
        if (sock && sock->id == 0) {
            if (sock->_cb) {
                sock->_cb(sock->_data);
            }
            break;
        }
    }
}

nsapi_error_t UNISOC_RDA8908A_CellularStack::socket_stack_init()
{
    char apn[MAX_ACCESSPOINT_NAME_LENGTH];
    int apn_len = 0;

    _at.lock();
    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START ", __FUNCTION__, __LINE__);

    _at.cmd_start("AT+CIPMUX=1");   // enable multi-ip connection
    _at.cmd_stop_read_resp();

    _at.cmd_start("AT+CIPQSEND=0");   // set transmitting mode to normal
    _at.cmd_stop_read_resp();

    _at.cmd_start("AT+CIPSRIP=1");  // enable to show remote IP and Port while receive data
    _at.cmd_stop_read_resp();

    _at.cmd_start("AT+CIPRXGET=1"); // enable to get data from network manually
    _at.cmd_stop_read_resp();

    _at.cmd_start("AT+CSTT?");
    _at.cmd_stop();
    _at.resp_start("+CSTT:");
    if (_at.info_resp()) {
        apn_len = _at.read_string(apn, sizeof(apn));
        _at.skip_param();
        _at.skip_param();
    }
    _at.resp_stop();
    if(!apn_len){
        _at.unlock();
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u: Not authenticated.", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    _at.cmd_start("AT+CSTT=");      // Start Task And Set APN, User ID, Password
    _at.write_string(apn);
    _at.write_string("");
    _at.write_string("");
    _at.cmd_stop_read_resp();

    _at.cmd_start("AT+CIICR");       // AT+CIICR Bring Up Wireless Connection With GPRS
    _at.cmd_stop_read_resp();

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END ", __FUNCTION__, __LINE__);
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
    int sock_close;

    _at.cmd_start("AT+CIPCLOSE=");
    _at.write_int(sock_id);
    _at.cmd_stop();

    _at.resp_start();
    sock_close = _at.read_int();
    _at.set_stop_tag("CLOSE OK");
    _at.resp_stop();

    tr_info("Close socket: %d error: %d", sock_id, _at.get_last_error());

    return _at.get_last_error();
}

void UNISOC_RDA8908A_CellularStack::handle_open_socket_response(int &modem_connect_id, int &err)
{
    char status[20];
    int status_len=0;

    nsapi_error_t ret_val = NSAPI_ERROR_OK;
    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START", __FUNCTION__, __LINE__);

    _at.set_at_timeout(5000);
    _at.resp_start();
    _at.set_stop_tag("\r\n");
    modem_connect_id = _at.read_int();
    status_len = _at.read_string(status, sizeof(status));
    _at.resp_stop();
    _at.restore_at_timeout();

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END [%s, %d, %d]", __FUNCTION__, __LINE__, status, err, ret_val);

    if (!strcmp(status, "CONNECT OK"))
        err = 0;
    else if(!strcmp(status, "ALREADY CONNECT"))
        err = 1;   

    ret_val = _at.get_last_error();

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END [%s, %d, %d]", __FUNCTION__, __LINE__, status, err, ret_val);
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
    int sent_len = -1;
    int sent_socket = -1;

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START [%d-%d]", __FUNCTION__, __LINE__,size,sent_len);

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

    //_at.set_debug(false);
    _at.cmd_start("AT+CIPSEND=");
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();
    _at.resp_start(">",true);
    _at.write_bytes((uint8_t *)data, size);

    _at.resp_start();
    sent_socket = _at.read_int();
    _at.set_stop_tag("SEND OK");
    _at.resp_stop();
    //_at.set_debug(true);

    if ((_at.get_last_error() != NSAPI_ERROR_OK)||(sent_socket!=socket->id)) {
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_DEVICE_ERROR]", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END", __FUNCTION__, __LINE__);
    return size;
}

nsapi_size_or_error_t UNISOC_RDA8908A_CellularStack::socket_recvfrom_impl(CellularSocket *socket, SocketAddress *address,
                                                                       void *buffer, nsapi_size_t size)
{
    nsapi_size_or_error_t recv_len = 0;
    int port;
    int mode = 0;
    char ip_address[NSAPI_IP_SIZE];

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START [%d]", __FUNCTION__, __LINE__,size);

    if ((socket == NULL)||(buffer==NULL)||(size==0)) {
        tr_error("UNISOC_RDA8908A_CellularStack:%s:%u:[NSAPI_ERROR_PARAMETER]", __FUNCTION__, __LINE__);
        return NSAPI_ERROR_PARAMETER;
    }
    //_at.set_debug(false);
    _at.cmd_start("AT+CIPRXGET=");
    _at.write_int(2);
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();
    _at.resp_start("+CIPRXGET:");
    if (_at.info_resp()) {
        mode = _at.read_int();
        if(mode){
            _at.skip_param();
            _at.skip_param();
            recv_len = _at.read_int();
            //_at.skip_param();
            if (recv_len > 0) {
                 _at.set_delimiter(':');
                _at.read_string(ip_address, sizeof(ip_address));
                port = _at.read_int();
                _at.set_default_delimiter();
                _at.read_bytes((uint8_t *)buffer, recv_len);
            }
            _at.resp_stop();
        }
    }

    if(!mode){
        tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: START [%d]", __FUNCTION__, __LINE__,size);
        _at.resp_start("+CIPRXGET:");
        mode = _at.read_int();
        if(mode){
            _at.skip_param();
            _at.skip_param();
            recv_len = _at.read_int();
            if (recv_len > 0) {
                _at.set_delimiter(':');
                _at.read_string(ip_address, sizeof(ip_address));
                port = _at.read_int();
                _at.set_default_delimiter();
                _at.read_bytes((uint8_t *)buffer, recv_len);
            }
            _at.resp_stop();
        }
    }
    //_at.set_debug(true);

    if (!recv_len || (_at.get_last_error() != NSAPI_ERROR_OK)||!mode) {
        tr_info("UNISOC_RDA8908A_CellularStack:%s:%u: BLOCK [%d]", __FUNCTION__, __LINE__,recv_len);
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    if (address) {
        address->set_ip_address(ip_address);
        address->set_port(port);
    }

    tr_debug("UNISOC_RDA8908A_CellularStack:%s:%u: END [%d, %d,%s,%d]", __FUNCTION__, __LINE__,mode, size,ip_address,port);
    return recv_len;
}