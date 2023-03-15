#include "mfs.h"
#include "udp.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>

static int fd;
static struct sockaddr_in saddr;

// int sendServer(char* message, char* respond) {
//     int send_rc = UDP_Write(sock_fd, &saddr, message, sizeof(message_t));
//     int rcv_rc = UDP_Read(sock_fd, &saddr, respond, sizeof(message_t));
//     return rcv_rc;
// }

int sendServer(message_t* send, message_t* response) {
    fd_set readfds;
    struct timeval timeVal;
    timeVal.tv_sec = 3;
    timeVal.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    int rc = UDP_Write(fd, &saddr, (char*)send, sizeof(message_t));
    if (rc < 0)return -1;
    if (select(fd + 1, &readfds, NULL, NULL, &timeVal)) {
        rc = UDP_Read(fd, &saddr, (char*)response, sizeof(message_t));
        if (rc < 0)return -1;

    }
    else return -1;
    return 0;

}

int MFS_Init(char* hostname, int port) {
    int MIN_PORT = 20000;
    int MAX_PORT = 40000;

    srand(time(0));
    int port_num = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);
    fd = UDP_Open(port_num);
    if (fd < 0)
        return fd;
    int rc = UDP_FillSockAddr(&saddr, hostname, port);
    if (rc < 0)
        return rc;

    return 0;
}

int MFS_Lookup(int pinum, char* name) {
    if (strlen(name) > 27)
        return -1;
    message_t message, respond;
    message.msg_type = MFS_LOOKUP;
    message.inum = pinum;
    strcpy(message.buf, name);
    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_LOOKUP)
        return -1;
    if (respond.inum < 0)
        return -1;

    return respond.inum;
}

int MFS_Stat(int inum, MFS_Stat_t* m) {
    message_t message, respond;
    message.msg_type = MFS_STAT;
    message.inum = inum;
    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_STAT)
        return -1;
    if (respond.inum < 0)
        return -1;
    memcpy(m, respond.buf, sizeof(MFS_Stat_t));

    return 0;
}

int MFS_Write(int inum, char* buffer, int offset, int nbytes) {
    if (nbytes > MFS_BUFFER)
        return -1;
    message_t message, respond;
    message.msg_type = MFS_WRITE;
    message.inum = inum;
    memcpy((char*)message.buf, buffer, nbytes);
    message.offset = offset;
    message.nbytes = nbytes;
    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_WRITE)
        return -1;
    if (respond.inum < 0)
        return -1;

    return 0;
}

int MFS_Read(int inum, char* buffer, int offset, int nbytes) {
    if (nbytes > MFS_BUFFER)
        return -1;
    message_t message, respond;
    message.msg_type = MFS_READ;
    message.inum = inum;
    message.offset = offset;
    message.nbytes = nbytes;
    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_READ)
        return -1;
    if (respond.inum < 0)
        return -1;
    memcpy(buffer, respond.buf, nbytes);

    return 0;
}

int MFS_Creat(int pinum, int type, char* name) {
    if (strlen(name) > 27)
        return -1;
    message_t message, respond;
    message.msg_type = MFS_CREATE;
    message.inum = pinum;
    strcpy(message.buf, name);
    message.creat_type = type;

    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_CREATE)
        return -1;
    if (respond.inum < 0)
        return -1;

    return 0;
}

int MFS_Unlink(int pinum, char* name) {
    if (strlen(name) > 27)
        return -1;
    message_t message, respond;
    message.msg_type = MFS_UNLINK;
    message.inum = pinum;
    strcpy(message.buf, name);
    if (sendServer(&message, &respond) < 0)
        return -1;
    if (respond.msg_type != MFS_UNLINK)
        return -1;
    if (respond.inum < 0)
        return -1;

    return 0;
}

int MFS_Shutdown() {
    message_t message, respond;
    message.msg_type = MFS_SHUTDOWN;
    if (sendServer(&message, &respond) < 0)
        return 0;
    if (respond.msg_type != MFS_SHUTDOWN)
        return -1;
    UDP_Close(fd);

    return 0;
}
