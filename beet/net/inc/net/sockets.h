#ifndef BEETROOT_SOCKETS_H
#define BEETROOT_SOCKETS_H

#include <cstdint>
//===defines=================
#define UDP_SOCKET_BUF_LEN 64

//===api=====================
void socket_open_udp();
void socket_set_address_client(uint16_t port, const char address[8] );
void socket_set_address_server(uint16_t port);

void socket_bind();
void socket_send(const char *in_buffer, const int& in_bufferSize);
void socket_receive_blocking(char *out_buffer, const int& in_bufferSize);

void socket_close();

//===init & shutdown=========
void socket_create();
void socket_cleanup();

#endif //BEETROOT_SOCKETS_H
