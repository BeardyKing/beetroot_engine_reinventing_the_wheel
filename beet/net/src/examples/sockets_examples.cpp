#include <net/sockets.h>
#include <net/examples/sockets_examples.h>

#include <cstdio>
#include <cstring>

void socket_example_update_client() {
    char message[UDP_SOCKET_BUF_LEN];
    printf("Enter message : \n");
    fgets(message, UDP_SOCKET_BUF_LEN, stdin);

    socket_send(message, UDP_SOCKET_BUF_LEN);
    socket_receive_blocking(message, UDP_SOCKET_BUF_LEN);
}

void socket_example_update_server() {
    char buf[UDP_SOCKET_BUF_LEN];
    memset(buf, '\0', UDP_SOCKET_BUF_LEN);
    socket_receive_blocking(buf, UDP_SOCKET_BUF_LEN);
    socket_send(buf, UDP_SOCKET_BUF_LEN);
}

void socket_example_create_client() {
    socket_create();
    socket_open_udp();
    socket_set_address_client(8888, "127.0.0.1");
}

void socket_example_cleanup_client() {
    socket_close();
    socket_cleanup();
}

void socket_example_create_server() {
    socket_create();
    socket_open_udp();
    socket_set_address_server(8888);
    socket_bind();
}

void socket_example_cleanup_server() {
    socket_close();
    socket_cleanup();
}
