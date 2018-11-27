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

#include "QUECTEL/M26/QUECTEL_M26_CellularStack.h"
#include "CellularLog.h"

using namespace mbed;

QUECTEL_M26_CellularStack::QUECTEL_M26_CellularStack(ATHandler &atHandler, int cid, nsapi_ip_stack_t stack_type) : AT_CellularStack(atHandler, cid, stack_type)
{
    _at.set_urc_handler("+QIRDI:", mbed::Callback<void()>(this, &QUECTEL_M26_CellularStack::urc_qiurc));
}

QUECTEL_M26_CellularStack::~QUECTEL_M26_CellularStack()
{
}

nsapi_error_t QUECTEL_M26_CellularStack::socket_listen(nsapi_socket_t handle, int backlog)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_error_t QUECTEL_M26_CellularStack::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

nsapi_error_t QUECTEL_M26_CellularStack::socket_bind(nsapi_socket_t handle, const SocketAddress &addr)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

void QUECTEL_M26_CellularStack::urc_qiurc()
{
    int sock_id = 0;

    _at.lock();
    (void) _at.skip_param(); /*<id> AT+QIFGCNT*/
    (void) _at.skip_param(); /*<sc> 1 Client, 2 Server*/
    sock_id = _at.read_int();
    (void) _at.skip_param(); /*<num>*/
    (void) _at.skip_param(); /*<len>*/
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

nsapi_error_t QUECTEL_M26_CellularStack::socket_stack_init()
{
    int tcpip_mode = 1;
    int mux_mode = 0;
    int cache_mode = 0;

    _at.lock();
    _at.cmd_start("AT+QIFGCNT=0");
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();

    /********************/
    _at.cmd_start("AT+QIMODE?");
    _at.cmd_stop();
    _at.resp_start("+QIMODE:");
    if (_at.info_resp()) {
        tcpip_mode = _at.read_int();
        tr_info("The tcpip_mode %d",tcpip_mode);
    }
    _at.resp_stop();
    /********************/
    if(tcpip_mode){
        _at.cmd_start("AT+QIMOD=0");
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();
    }
    /********************/
    _at.cmd_start("AT+QIMUX?");
    _at.cmd_stop();
    _at.resp_start("+QIMUX:");
    if (_at.info_resp()) {
        mux_mode = _at.read_int();
        tr_info("The multiplexer mode %d",mux_mode);
    }
    _at.resp_stop();

    if(!mux_mode){
        _at.cmd_start("AT+QIMUX=1");
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();
    }
    /********************/
    _at.cmd_start("AT+QINDI?");
    _at.cmd_stop();
    _at.resp_start();
    if (_at.info_resp()) {
        cache_mode = _at.read_int();
        tr_info("The cache_mode mode %d",cache_mode);
    }
    _at.resp_stop();

    if(cache_mode!=2){
        _at.cmd_start("AT+QINDI=2");
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();
    }

    return _at.unlock_return_error();
}

int QUECTEL_M26_CellularStack::get_max_socket_count()
{
    return M26_SOCKET_MAX;
}

bool QUECTEL_M26_CellularStack::is_protocol_supported(nsapi_protocol_t protocol)
{
    return (protocol == NSAPI_UDP || protocol == NSAPI_TCP);
}

nsapi_error_t QUECTEL_M26_CellularStack::socket_close_impl(int sock_id)
{
    _at.cmd_start("AT+QICLOSE=");
    _at.write_int(sock_id);
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();

    return _at.get_last_error();
}

void QUECTEL_M26_CellularStack::handle_open_socket_response(int &modem_connect_id, int &err)
{
    char status[15];
    printf("### handle_open_socket_response ### 1");
    _at.resp_start();
    if(_at.info_resp()||_at.get_last_error())
    {
        if(_at.info_resp()){
            printf("### OPEN RESPONSE ###: ALREADY CONNECT true");
            _at.read_string(status, sizeof(status));
        }
        else{
            printf("### OPEN RESPONSE ###: ERROR");
        }
        _at.resp_stop();
        err = 1;
        return;
    }

    _at.resp_stop();
    printf("### handle_open_socket_response ### 2");
    _at.set_at_timeout(M26_CREATE_SOCKET_TIMEOUT);
    _at.resp_start();
    modem_connect_id = _at.read_int();
    int len = _at.read_string(status, sizeof(status));
    _at.resp_stop();
    _at.restore_at_timeout();
    err = (_at.get_last_error() != NSAPI_ERROR_OK)?1:0;

    printf("###handle_open_socket_response ### 3: %s, %d, %d , %d\n",status, len, err, sizeof(status));
}
nsapi_error_t QUECTEL_M26_CellularStack::create_socket_impl(CellularSocket *socket)
{
    int modem_connect_id = -1;
    int request_connect_id = socket->id;
    int err = -1;

    tr_info("##### QUECTEL_M26_CellularStack::create_socket_impl %d,%d\n",socket->proto,socket->connected);

    if (socket->connected) {
        _at.cmd_start("AT+QIOPEN=");
        //_at.write_int(_cid);
        _at.write_int(request_connect_id);

        if(socket->proto == NSAPI_TCP)
            _at.write_string("TCP");
        else
            _at.write_string("UDP");

        _at.write_string(socket->remoteAddress.get_ip_address());
        _at.write_int(socket->remoteAddress.get_port());
        _at.cmd_stop();

        handle_open_socket_response(modem_connect_id, err);

        if ((_at.get_last_error() != NSAPI_ERROR_OK) || err) {
            _at.cmd_start("AT+QICLOSE=");
            _at.write_int(modem_connect_id);
            _at.cmd_stop();
            _at.resp_start();
            _at.resp_stop();

            _at.cmd_start("AT+QIOPEN=");
            _at.write_int(request_connect_id);
            if(socket->proto == NSAPI_TCP)
                _at.write_string("TCP");
            else
                _at.write_string("UDP");

            _at.write_string(socket->remoteAddress.get_ip_address());
            _at.write_int(socket->remoteAddress.get_port());
            _at.cmd_stop();

            handle_open_socket_response(modem_connect_id, err);
        }
    }

    // If opened successfully BUT not requested one, close it
    if (!err && (modem_connect_id != request_connect_id)) {
        _at.cmd_start("AT+QICLOSE=");
        _at.write_int(modem_connect_id);
        _at.cmd_stop();
        _at.resp_start();
        _at.resp_stop();
    }

    nsapi_error_t ret_val = _at.get_last_error();

    socket->created = ((ret_val == NSAPI_ERROR_OK) && (modem_connect_id == request_connect_id));

    tr_info("##### socket->created = %d, ret_val=%d",socket->created,ret_val);
    return ret_val;
}

nsapi_size_or_error_t QUECTEL_M26_CellularStack::socket_sendto_impl(CellularSocket *socket, const SocketAddress &address,
                                                                     const void *data, nsapi_size_t size)
{
    int sent_len = 0;
    int sent_acked = 0;
    int sent_nacked = 0;

    int sent_len_before = 0;
    int sent_len_after = 0;

    if((size==0)||(size > 1460 )){
        return NSAPI_ERROR_PARAMETER;
    }

    if(!socket->created){
        socket->remoteAddress = address;
        socket->connected = true;
        nsapi_error_t ret_val = create_socket_impl(socket);
        if((ret_val!= NSAPI_ERROR_OK)||(!socket->created))
            return NSAPI_ERROR_NO_SOCKET;
    }

    tr_info("#IN#### socket_sendto_impl send size  = %d ##########################\n",size);

    _at.set_debug(false);

    _at.cmd_start("AT+QISACK=");
    _at.write_int(socket->id);
    _at.cmd_stop();

    _at.resp_start("+QISACK:");
    sent_len_before = _at.read_int();
    sent_acked = _at.read_int();
    sent_nacked = _at.read_int();
    _at.resp_stop();

    _at.cmd_start("AT+QISEND=");
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();

    _at.resp_start(">");
    _at.write_bytes((uint8_t *)data, size);
    _at.resp_start();
    _at.resp_stop();

    _at.cmd_start("AT+QISACK=");
    _at.write_int(socket->id);
    _at.cmd_stop();

    _at.resp_start("+QISACK:");
    sent_len = _at.read_int();
    sent_acked = _at.read_int();
    sent_nacked = _at.read_int();
    _at.resp_stop();

    _at.set_debug(true);
    if (_at.get_last_error() == NSAPI_ERROR_OK) {
        tr_info("#OUT#### socket_sendto_impl: %d %d %d ##########################\n",sent_len_after,sent_acked,sent_nacked);
        if(sent_acked == size) return size;
        sent_len = sent_len_after - sent_len_before;
        return sent_len;
    }

    nsapi_error_t error = _at.get_last_error();
    tr_info("#OUT#### socket_sendto_impl error  = %d ##########################\n",error);
    return error;
}

nsapi_size_or_error_t QUECTEL_M26_CellularStack::socket_recvfrom_impl(CellularSocket *socket, SocketAddress *address,
                                                                       void *buffer, nsapi_size_t size)
{
    nsapi_size_or_error_t recv_len = 0;
    int port;
    char type[8];
    char ip_address[NSAPI_IP_SIZE + 1];
    tr_info("#IN#### socket_recvfrom_impl read size  = %d ##########################\n",size);
    _at.set_debug(false);
    _at.set_at_timeout(4000);

    _at.cmd_start("AT+QIRD=");
    _at.write_int(0);//at+qifgcnt 0-1
    _at.write_int(1);//1-Client, 2-Server
    _at.write_int(socket->id);
    _at.write_int(size);
    _at.cmd_stop();

    _at.resp_start("+QIRD:");
    if (_at.info_resp()) {
        _at.set_delimiter(':');
        _at.read_string(ip_address, sizeof(ip_address));
        _at.set_default_delimiter();
        port = _at.read_int();
        _at.read_string(type,sizeof(type));
        recv_len = _at.read_int();
        if (recv_len > 0) {
            _at.read_bytes((uint8_t *)buffer, recv_len);
        }
    }
    _at.resp_stop();
    _at.set_debug(true);
    _at.restore_at_timeout();

    if (recv_len > 0) {
        //_at.read_bytes((uint8_t *)buffer, recv_len);
        /*
        printf("\n==============================================================================\n");
        for(int i=0;i<recv_len;i++){
            printf("(%x)",((unsigned char *)buffer)[i]);
            if(i%16==15)printf("\n");
        }
        */
        //printf("\n-----------------------------------------------------------------------------\n");
        tr_info("#OUT#### socket_recvfrom_impl: recv_len = %d,%d,%s,%s\n",recv_len,port,ip_address,type);
        //printf("\n==============================================================================\n");
    }

    if (!recv_len || ( _at.get_last_error()!= NSAPI_ERROR_OK)) {
        //tr_info("#OUT#### socket_recvfrom_impl recv_len = %d\n",recv_len);
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    if (address) {
        address->set_ip_address(ip_address);
        address->set_port(port);
    }

    //tr_info("#OUT#### socket_recvfrom_impl recv_len = %d\n",recv_len);
    return recv_len;
}
