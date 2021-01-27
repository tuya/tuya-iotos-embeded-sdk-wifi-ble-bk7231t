/**
 * @file tuya_hal_network.h
 * @author maht@tuya.com
 * @brief 网络接口封装文件
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */


#ifndef __TUYA_HAL_NETWORK_H__
#define __TUYA_HAL_NETWORK_H__


#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
    extern "C" {
#endif


/* tuya sdk definition of 127.0.0.1 */
#define TY_IPADDR_LOOPBACK     ((uint32_t)0x7f000001UL)
/* tuya sdk definition of 0.0.0.0 */
#define TY_IPADDR_ANY          ((uint32_t)0x00000000UL)
/* tuya sdk definition of 255.255.255.255 */
#define TY_IPADDR_BROADCAST    ((uint32_t)0xffffffffUL)

/* tuya sdk definition of socket protocol */
typedef enum {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1,
}UNW_PROTOCOL_TYPE;

/* tuya sdk definition of transfer type */
typedef enum {
    TRANS_RECV = 0,
    TRANS_SEND = 1,
}UNW_TRANS_TYPE_E;

/* tuya sdk definition of shutdown type */
#define UNW_SHUT_RD   0
#define UNW_SHUT_WR   1
#define UNW_SHUT_RDWR 2

/* tuya sdk definition of socket errno */
typedef int UNW_ERRNO_T;
#define UNW_SUCCESS       0
#define UNW_FAIL          -1
#define UNW_EINTR         -2
#define UNW_EBADF         -3
#define UNW_EAGAIN        -4
#define UNW_EFAULT        -5
#define UNW_EBUSY         -6
#define UNW_EINVAL        -7
#define UNW_ENFILE        -8
#define UNW_EMFILE        -9
#define UNW_ENOSPC        -10
#define UNW_EPIPE         -11
#define UNW_EWOULDBLOCK   -12
#define UNW_ENOTSOCK      -13
#define UNW_ENOPROTOOPT   -14
#define UNW_EADDRINUSE    -15
#define UNW_EADDRNOTAVAIL -16
#define UNW_ENETDOWN      -17
#define UNW_ENETUNREACH   -18
#define UNW_ENETRESET     -19
#define UNW_ECONNRESET    -20
#define UNW_ENOBUFS       -21
#define UNW_EISCONN       -22
#define UNW_ENOTCONN      -23
#define UNW_ETIMEDOUT     -24
#define UNW_ECONNREFUSED  -25
#define UNW_EHOSTDOWN     -26
#define UNW_EHOSTUNREACH  -27
#define UNW_ENOMEM        -28
#define UNW_EMSGSIZE      -29

/* fd 最大个数, 不同平台根据实际情况定义 */
#define UNW_FD_MAX_COUNT    (64)

/* tuya sdk definition of fd operations */
typedef struct {
    uint8_t placeholder[(UNW_FD_MAX_COUNT+7)/8];
} UNW_FD_SET_T;

#define UNW_FD_SET(n,p)     tuya_hal_net_fd_set(n, p)
#define UNW_FD_CLR(n, p)    tuya_hal_net_fd_clear(n, p)
#define UNW_FD_ISSET(n,p)   tuya_hal_net_fd_isset(n,p)
#define UNW_FD_ZERO(p)      tuya_hal_net_fd_zero(p)

/* tuya sdk definition of IP info */
typedef struct
{
    char ip[16];    /* ip addr:  xxx.xxx.xxx.xxx  */
    char mask[16];  /* net mask: xxx.xxx.xxx.xxx  */
    char gw[16];    /* gateway:  xxx.xxx.xxx.xxx  */
}NW_IP_S;

/* tuya sdk definition of MAC info */
typedef struct
{
    uint8_t mac[6]; /* mac address */
}NW_MAC_S;

/* tuya sdk definition of IP addr */
typedef uint32_t UNW_IP_ADDR_T;

/***********************************************************
*  Function: unw_get_errno
*  Desc:     tuya sdk definition of errno
*  Return:   tuya sdk definition of socket errno
***********************************************************/
UNW_ERRNO_T tuya_hal_net_get_errno(void);

UNW_IP_ADDR_T tuya_hal_net_addr(const char *cp);

int tuya_hal_net_fd_set(int fd, UNW_FD_SET_T* fds);
int tuya_hal_net_fd_clear(int fd, UNW_FD_SET_T* fds);
int tuya_hal_net_fd_isset(int fd, UNW_FD_SET_T* fds);
int tuya_hal_net_fd_zero(UNW_FD_SET_T* fds);

/***********************************************************
*  Function: unw_select
*  Desc:     tuya sdk definition of socket select
*  Input && Output && Return: refer to std select
***********************************************************/
int tuya_hal_net_select(const int maxfd, UNW_FD_SET_T *readfds, UNW_FD_SET_T *writefds,
               UNW_FD_SET_T *errorfds, const uint32_t ms_timeout);

/***********************************************************
*  Function: unw_get_nonblock
*  Desc:     check where a socket fd is blocked
*  Input:    fd: socket fd
*  Return:   <0: fail  >0: non-block  0: block
***********************************************************/
int tuya_hal_net_get_nonblock(const int fd);

/***********************************************************
*  Function: unw_set_block
*  Desc:     set the socket fd to block/non-block state
*  Input:    fd: socket fd  block: the new state
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_block(const int fd, const bool block);

/***********************************************************
*  Function: unw_close
*  Desc:     tuya sdk definition of socket close
*  Input && Output && Return: refer to std close
***********************************************************/
int tuya_hal_net_close(const int fd);

/***********************************************************
*  Function: unw_shutdown
*  Desc:     tuya sdk definition of socket shutdown
*  Input && Output && Return: refer to std shutdown
***********************************************************/
int tuya_hal_net_shutdown(const int fd, const int how);

/***********************************************************
*  Function: unw_socket_create
*  Desc:     create a tcp/udp socket
*  Input:    type: tcp/udp type
*  Return: refer to std socket
***********************************************************/
int tuya_hal_net_socket_create(const UNW_PROTOCOL_TYPE type);

/***********************************************************
*  Function: unw_connect
*  Desc:     connect the socket fd to a addr and a port
*  Return:   refer to std connect
***********************************************************/
int tuya_hal_net_connect(const int fd, const UNW_IP_ADDR_T addr, const uint16_t port);

/***********************************************************
*  Function: unw_connect_raw
*  Desc:     tuya sdk definition of socket connect
*  Input && Output && Return: refer to std connect
***********************************************************/
int tuya_hal_net_connect_raw(const int fd, void *p_socket, const int len);

/***********************************************************
*  Function: unw_bind
*  Desc:     bind the socket fd to a addr and a port
*  Return:   refer to std bind
***********************************************************/
int tuya_hal_net_bind(const int fd, const UNW_IP_ADDR_T addr, const uint16_t port);

/***********************************************************
*  Function: unw_listen
*  Desc:     tuya sdk definition of socket listen
*  Input && Output && Return: refer to std listen
***********************************************************/
int tuya_hal_net_listen(const int fd, const int backlog);

/***********************************************************
*  Function: unw_send
*  Desc:     tuya sdk definition of socket send
*  Input && Output && Return: refer to std send
***********************************************************/
int tuya_hal_net_send(const int fd, const void *buf, const uint32_t nbytes);

/***********************************************************
*  Function: unw_send_to
*  Desc:     tuya sdk definition of socket sendto
*  Input && Output && Return: refer to std sendto
***********************************************************/
int tuya_hal_net_send_to(const int fd, const void *buf, const uint32_t nbytes,
                const UNW_IP_ADDR_T addr, const uint16_t port);

/***********************************************************
*  Function: unw_recv
*  Desc:     tuya sdk definition of socket recv
*  Input && Output && Return: refer to std recv
***********************************************************/
int tuya_hal_net_recv(const int fd, void *buf, const uint32_t nbytes);

/***********************************************************
*  Function: unw_recvfrom
*  Desc:     tuya sdk definition of socket recvfrom
*  Input && Output && Return: refer to std recvfrom
***********************************************************/
int tuya_hal_net_recvfrom(const int fd, void *buf, const uint32_t nbytes,
                 UNW_IP_ADDR_T *addr, uint16_t *port);

/***********************************************************
*  Function: unw_set_timeout
*  Desc:     set socket fd timeout option
*  Input:    fd: socket fd
*  Input:    ms_timeout: timeout in ms
*  Input:    type: transfer type
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_timeout(const int fd, const int ms_timeout,
                    const UNW_TRANS_TYPE_E type);

/***********************************************************
*  Function: unw_set_bufsize
*  Desc:     set socket fd buffer_size option
*  Input:    fd: socket fd
*  Input:    buf_size: buffer size in byte
*  Input:    type: transfer type
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_bufsize(const int fd, const int buf_size,
                    const UNW_TRANS_TYPE_E type);

/***********************************************************
*  Function: unw_set_reuse
*  Desc:     enable socket fd reuse option
*  Input:    fd: socket fd
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_reuse(const int fd);

/***********************************************************
*  Function: unw_disable_nagle
*  Desc:     disable socket fd nagle option
*  Input:    fd: socket fd
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_disable_nagle(const int fd);

/***********************************************************
*  Function: unw_set_boardcast
*  Desc:     enable socket broadcast option
*  Input:    fd: socket fd
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_boardcast(const int fd);

/***********************************************************
*  Function: unw_gethostbyname
*  Desc:     change the domain to addr info
*  Input:    domain: domin info
*  Output:   addr: addr info
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_gethostbyname(const char *domain, UNW_IP_ADDR_T *addr);

/***********************************************************
*  Function: unw_accept
*  Desc:     accept the coming socket connect of the server fd
*  Input:    fd: the server fd
*  Output:   addr && port: the coming addr info && port
*  Return:   >0: the coming socket fd.   others: fail
***********************************************************/
int tuya_hal_net_accept(const int fd, UNW_IP_ADDR_T *addr, uint16_t *port);

/***********************************************************
*  Function: unw_recv_nd_size
*  Desc:     recv <nd_size> from the socket fd, store in the <buf>.
*  Input:    fd: the socket fd
*  Input:    buf && buf_size: the buffer info
*  Input:    nd_size: the need size
*  Output:   buf: the content recv from socket fd
*  Return:   >0: success   others: fail
***********************************************************/
int tuya_hal_net_recv_nd_size(const int fd, void *buf, 
                     const uint32_t buf_size, const uint32_t nd_size);

UNW_IP_ADDR_T tuya_hal_net_str2addr(char *ip_str);

/***********************************************************
*  Function: unw_set_keepalive
*  Desc:     set the socket option:SO_KEEPALIVE TCP_KEEPIDLE TCP_KEEPINTVL TCP_KEEPCNT
*  Input:    fd: the socket fd
*  Input:    alive && idle && intr && cnt: options
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_keepalive(int fd, const bool alive,
                              const uint32_t idle, const uint32_t intr,
                              const uint32_t cnt);

/***********************************************************
*  Function: unw_socket_bind
*  Desc:     bind the socket fd to a ip
*  Input:    fd: the socket fd
*  Input:    ip: ip addr
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_socket_bind(int fd, const char *ip);

/***********************************************************
*  Function: unw_set_cloexec
*  Desc:     enable socket fd CLOEXEC
*  Input:    fd: the socket fd
*  Return:   UNW_SUCCESS: success   others: fail
***********************************************************/
int tuya_hal_net_set_cloexec(const int fd);

#ifdef __cplusplus
}
#endif

#endif // __TUYA_HAL_NETWORK_H__

