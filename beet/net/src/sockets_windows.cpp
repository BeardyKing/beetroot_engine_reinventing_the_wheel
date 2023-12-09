#if defined(_WIN32)

#include <net/sockets.h>

#include<winsock2.h>

#pragma comment(lib, "ws2_32.lib") //Winsock Library

#include <shared/assert.h>
#include <shared/log.h>

//===internal structs========
SOCKET s;
struct sockaddr_in si_other;
struct sockaddr_in server;

//===internal functions======
void log_packet_info(const char *in_buffer, const int &in_bufferSize, const sockaddr_in &info) {
    printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    char data[UDP_SOCKET_BUF_LEN];
    strcpy_s(data, sizeof(char) * in_bufferSize, in_buffer);
    log_verbose(MSG_NET,"Data: %s\n", data);
}

//===api=====================
void socket_send(const char *in_buffer, const int &in_bufferSize) {
    int slen = sizeof(si_other);
    int err = sendto(s, in_buffer, in_bufferSize, 0, (struct sockaddr *) &si_other, slen);
    ASSERT_MSG(!(err == SOCKET_ERROR), "sendto failed with error code : %d", WSAGetLastError());
}

void socket_receive_blocking(char *out_buffer, const int &in_bufferSize) {
    int slen = sizeof(si_other);
    memset(out_buffer, '\0', in_bufferSize);

    int err = recvfrom(s, out_buffer, in_bufferSize, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR;
    ASSERT_MSG(!(err == SOCKET_ERROR), "recvfrom failed with error code : %d", WSAGetLastError());

#if BEET_DEBUG
    log_packet_info(out_buffer, in_bufferSize, si_other);
#endif
}

void socket_open_udp() {
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ASSERT_MSG(!(s == SOCKET_ERROR), "socket failed with error code : %d", WSAGetLastError());
}

void socket_bind() {
    int32_t err = bind(s, (struct sockaddr *) &server, sizeof(server));
    ASSERT_MSG(err == SOCKET_ERROR, "bind failed with error code : %d", WSAGetLastError());
}

void socket_set_address_client(uint16_t port, const char address[8]) {
    //setup address structure
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);
    si_other.sin_addr.S_un.S_addr = inet_addr(address);
}

void socket_set_address_server(uint16_t port) {
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
}

void socket_close() {
    closesocket(s);
}

//===init & shutdown=========
void socket_create() {
    WSADATA wsa;
    int32_t err = WSAStartup(MAKEWORD(2, 2), &wsa);// != 0) {
    ASSERT_MSG(!(err == INVALID_SOCKET), "bind failed with error code : %d", WSAGetLastError());
}

void socket_cleanup() {
    WSACleanup();
}


#endif