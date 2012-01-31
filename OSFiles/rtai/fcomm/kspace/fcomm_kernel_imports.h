/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/
#ifndef FCOMM_K_IMPORTS_H
#define	FCOMM_K_IMPORTS_H

#if defined(_RTAI)
    #include <stdarg.h>
    #include <sys/select.h>    
    typedef long long int __quad_t;
    typedef unsigned long long int __u_quad_t;    
    #define __S16_TYPE short int
    #define __U16_TYPE unsigned short int
    #define __S32_TYPE int
    #define __U32_TYPE unsigned int
    #define __SLONGWORD_TYPE long int
    #define __ULONGWORD_TYPE unsigned long int
    #define __SQUAD_TYPE __quad_t
    #define __UQUAD_TYPE __u_quad_t
    #define __SWORD_TYPE int
    #define __UWORD_TYPE unsigned int
    #define __SLONG32_TYPE long int
    #define __ULONG32_TYPE unsigned long int
    #define __S64_TYPE __quad_t
    #define __U64_TYPE __u_quad_t
    
    typedef __UQUAD_TYPE dev_t;
    typedef __ULONGWORD_TYPE ino_t;
    typedef __U32_TYPE uint32_t;
    typedef __U16_TYPE uint16_t;
    typedef __UQUAD_TYPE ssize_t;
    typedef uint32_t mode_t;
    typedef __SLONGWORD_TYPE off_t;
    typedef __SLONGWORD_TYPE time_t;
    typedef __UWORD_TYPE nlink_t;
    typedef __U32_TYPE uid_t;
    typedef __U32_TYPE gid_t;
    typedef __SLONGWORD_TYPE blksize_t;
    typedef __SLONGWORD_TYPE blkcnt_t;
    #define MSG_PEEK 2
    #define F_GETFL 3
    #define F_SETFL 4
    #define EACCES 13
    #define EINPROGRESS 115
    #define EWOULDBLOCK 11
    #define O_ACCMODE          0003
    #define O_RDONLY             00
    #define O_WRONLY             01
    #define O_RDWR               02
    #define O_CREAT            0100 /* not fcntl */
    #define O_EXCL             0200 /* not fcntl */
    #define O_NOCTTY           0400 /* not fcntl */
    #define O_TRUNC           01000 /* not fcntl */
    #define O_APPEND          02000
    #define O_NONBLOCK        04000
    #define O_NDELAY        O_NONBLOCK
    #define O_SYNC           010000
    #define O_FSYNC          O_SYNC
    #define O_ASYNC          020000
    #define LOCK_SH 1       /* Shared lock.  */
    #define LOCK_EX 2       /* Exclusive lock.  */
    #define LOCK_UN 8       /* Unlock.  */
    #define LOCK_NB 4       /* Don't block when locking.  */
    
    //32 bits size should be 88
    struct stat {        
        __dev_t st_dev;                     /* Device.  */
        unsigned short int __pad1;
        __ino_t st_ino;                     /* File serial number.  */
        __mode_t st_mode;                   /* File mode.  */
        __nlink_t st_nlink;                 /* Link count.  */
        __uid_t st_uid;                     /* User ID of the file's owner. */
        __gid_t st_gid;                     /* Group ID of the file's group.*/
        __dev_t st_rdev;                    /* Device number, if device.  */
        unsigned short int __pad2;
        __off_t st_size;                    /* Size of file, in bytes.  */
        __blksize_t st_blksize;             /* Optimal block size for I/O.  */
        __blkcnt_t st_blocks;               /* Number 512-byte blocks allocated. */
        struct timespec st_atim;            /* Time of last access.  */
        struct timespec st_mtim;            /* Time of last modification.  */
        struct timespec st_ctim;            /* Time of last status change.  */
        # define st_atime st_atim.tv_sec        /* Backward compatibility.  */
        # define st_mtime st_mtim.tv_sec
        # define st_ctime st_ctim.tv_sec
        unsigned long int __unused4;
        unsigned long int __unused5;
    };
    
    #define __S_IFMT        0170000 /* These bits determine file type.  */
    /* File types.  */
    #define __S_IFDIR       0040000 /* Directory.  */
    #define __S_IFCHR       0020000 /* Character device.  */
    #define __S_IFBLK       0060000 /* Block device.  */
    #define __S_IFREG       0100000 /* Regular file.  */
    #define __S_IFIFO       0010000 /* FIFO.  */
    #define __S_IFLNK       0120000 /* Symbolic link.  */
    #define __S_IFSOCK      0140000 /* Socket.  */
    #define __S_ISTYPE(mode, mask)  (((mode) & __S_IFMT) == (mask))

    #define S_ISDIR(mode)    __S_ISTYPE((mode), __S_IFDIR)
    #define S_ISCHR(mode)    __S_ISTYPE((mode), __S_IFCHR)
    #define S_ISBLK(mode)    __S_ISTYPE((mode), __S_IFBLK)
    #define S_ISREG(mode)    __S_ISTYPE((mode), __S_IFREG)
    #ifdef __S_IFIFO
        # define S_ISFIFO(mode)  __S_ISTYPE((mode), __S_IFIFO)
    #endif
    #ifdef __S_IFLNK
        # define S_ISLNK(mode)   __S_ISTYPE((mode), __S_IFLNK)
    #endif
#endif

struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file */
    char           d_name[256]; /* filename */
};    
    
    
#define SEEK_SET       0       /* Seek from beginning of file.  */
#define SEEK_CUR       1       /* Seek from current position.  */
#define SEEK_END       2
    
#define FIONBIO 0x5421
#define SCM_RIGHTS      0x01            /* rw: access rights (array of int) */
#define SCM_CREDENTIALS 0x02            /* rw: struct ucred             */
#define SCM_SECURITY    0x03            /* rw: security label           */

/* Supported address families. */
#define AF_UNSPEC       0
#define AF_UNIX         1       /* Unix domain sockets          */
#define AF_LOCAL        1       /* POSIX name for AF_UNIX       */
#define AF_INET         2       /* Internet IP Protocol         */
#define AF_AX25         3       /* Amateur Radio AX.25          */
#define AF_IPX          4       /* Novell IPX                   */
#define AF_APPLETALK    5       /* AppleTalk DDP                */
#define AF_NETROM       6       /* Amateur Radio NET/ROM        */
#define AF_BRIDGE       7       /* Multiprotocol bridge         */
#define AF_ATMPVC       8       /* ATM PVCs                     */
#define AF_X25          9       /* Reserved for X.25 project    */
#define AF_INET6        10      /* IP version 6                 */
#define AF_ROSE         11      /* Amateur Radio X.25 PLP       */
#define AF_DECnet       12      /* Reserved for DECnet project  */
#define AF_NETBEUI      13      /* Reserved for 802.2LLC project*/
#define AF_SECURITY     14      /* Security callback pseudo AF */
#define AF_KEY          15      /* PF_KEY key management API */
#define AF_NETLINK      16
#define AF_ROUTE        AF_NETLINK /* Alias to emulate 4.4BSD */
#define AF_PACKET       17      /* Packet family                */
#define AF_ASH          18      /* Ash                          */
#define AF_ECONET       19      /* Acorn Econet                 */
#define AF_ATMSVC       20      /* ATM SVCs                     */
#define AF_SNA          22      /* Linux SNA Project (nutters!) */
#define AF_IRDA         23      /* IRDA sockets                 */
#define AF_PPPOX        24      /* PPPoX sockets                */
#define AF_WANPIPE      25      /* Wanpipe API Sockets */
#define AF_LLC          26      /* Linux LLC                    */
#define AF_TIPC         30      /* TIPC sockets                 */
#define AF_BLUETOOTH    31      /* Bluetooth sockets            */
#define AF_MAX          32      /* For now.. */

/* Protocol families, same as address families. */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_LOCAL        AF_LOCAL
#define PF_INET         AF_INET
#define PF_AX25         AF_AX25
#define PF_IPX          AF_IPX
#define PF_APPLETALK    AF_APPLETALK
#define PF_NETROM       AF_NETROM
#define PF_BRIDGE       AF_BRIDGE
#define PF_ATMPVC       AF_ATMPVC
#define PF_X25          AF_X25
#define PF_INET6        AF_INET6
#define PF_ROSE         AF_ROSE
#define PF_DECnet       AF_DECnet
#define PF_NETBEUI      AF_NETBEUI
#define PF_SECURITY     AF_SECURITY
#define PF_KEY          AF_KEY
#define PF_NETLINK      AF_NETLINK
#define PF_ROUTE        AF_ROUTE
#define PF_PACKET       AF_PACKET
#define PF_ASH          AF_ASH
#define PF_ECONET       AF_ECONET
#define PF_ATMSVC       AF_ATMSVC
#define PF_SNA          AF_SNA
#define PF_IRDA         AF_IRDA
#define PF_PPPOX        AF_PPPOX
#define PF_WANPIPE      AF_WANPIPE
#define PF_LLC          AF_LLC
#define PF_TIPC         AF_TIPC
#define PF_BLUETOOTH    AF_BLUETOOTH
#define PF_MAX          AF_MAX

/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP          0
/* #define SOL_ICMP     1       No-no-no! Due to Linux :-) we cannot use SOL_ICMP=1 */
#define SOL_TCP         6
#define SOL_UDP         17
#define SOL_IPV6        41
#define SOL_ICMPV6      58
#define SOL_SCTP        132
#define SOL_RAW         255
#define SOL_IPX         256
#define SOL_AX25        257
#define SOL_ATALK       258
#define SOL_NETROM      259
#define SOL_ROSE        260
#define SOL_DECNET      261
#define SOL_X25         262
#define SOL_PACKET      263
#define SOL_ATM         264     /* ATM layer (cell level) */
#define SOL_AAL         265     /* ATM Adaption Layer (packet level) */
#define SOL_IRDA        266
#define SOL_NETBEUI     267
#define SOL_LLC         268
#define SOL_DCCP        269
#define SOL_NETLINK     270
#define SOL_TIPC        271

//THIS IS DIFFERENT FROM THE KERNEL DEFINITION, WHERE SOCK_STREAM is = 2. 
/* Types of sockets.  */
enum __socket_type
{
  SOCK_STREAM = 1,              /* Sequenced, reliable, connection-based
                                   byte streams.  */
#define SOCK_STREAM SOCK_STREAM
  SOCK_DGRAM = 2,               /* Connectionless, unreliable datagrams
                                   of fixed maximum length.  */
#define SOCK_DGRAM SOCK_DGRAM
  SOCK_RAW = 3,                 /* Raw protocol interface.  */
#define SOCK_RAW SOCK_RAW
  SOCK_RDM = 4,                 /* Reliably-delivered messages.  */
#define SOCK_RDM SOCK_RDM
  SOCK_SEQPACKET = 5,           /* Sequenced, reliable, connection-based,
                                   datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
  SOCK_PACKET = 10              /* Linux specific way of getting packets
                                   at the dev level.  For writing rarp and
                                   other similar things on the user level. */
#define SOCK_PACKET SOCK_PACKET
};

/* Address to accept any incoming messages. */
#define INADDR_ANY              ((unsigned long int) 0x00000000)

/* Address to send to all hosts. */
#define INADDR_BROADCAST        ((unsigned long int) 0xffffffff)

/* Address indicating an error return. */
#define INADDR_NONE             ((unsigned long int) 0xffffffff)

/* Network number for local host loopback. */
#define IN_LOOPBACKNET          127

/* Address to loopback in software to local host.  */
#define INADDR_LOOPBACK         0x7f000001      /* 127.0.0.1   */
#define IN_LOOPBACK(a)          ((((long int) (a)) & 0xff000000) == 0x7f000000)

/* For setsockopt(2) */
#define SOL_SOCKET      1

#define SO_DEBUG        1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_DONTROUTE    5
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_SNDBUFFORCE  32
#define SO_RCVBUFFORCE  33
#define SO_KEEPALIVE    9
#define SO_OOBINLINE    10
#define SO_NO_CHECK     11
#define SO_PRIORITY     12
#define SO_LINGER       13
#define SO_BSDCOMPAT    14
/* To add :#define SO_REUSEPORT 15 */
#define SO_PASSCRED     16
#define SO_PEERCRED     17
#define SO_RCVLOWAT     18
#define SO_SNDLOWAT     19
#define SO_RCVTIMEO     20
#define SO_SNDTIMEO     21

/* Security levels - as per NRL IPv6 - don't actually do anything */
#define SO_SECURITY_AUTHENTICATION              22
#define SO_SECURITY_ENCRYPTION_TRANSPORT        23
#define SO_SECURITY_ENCRYPTION_NETWORK          24

#define SO_BINDTODEVICE 25

/* Socket filtering */
#define SO_ATTACH_FILTER        26
#define SO_DETACH_FILTER        27

#define SO_PEERNAME             28
#define SO_TIMESTAMP            29
#define SCM_TIMESTAMP           SO_TIMESTAMP

#define SO_ACCEPTCONN           30

#define SO_PEERSEC              31
#define SO_PASSSEC              34


/* Standard well-defined IP protocols.  */
enum {
  IPPROTO_IP = 0,               /* Dummy protocol for TCP               */
  IPPROTO_ICMP = 1,             /* Internet Control Message Protocol    */
  IPPROTO_IGMP = 2,             /* Internet Group Management Protocol   */
  IPPROTO_IPIP = 4,             /* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,              /* Transmission Control Protocol        */
  IPPROTO_EGP = 8,              /* Exterior Gateway Protocol            */
  IPPROTO_PUP = 12,             /* PUP protocol                         */
  IPPROTO_UDP = 17,             /* User Datagram Protocol               */
  IPPROTO_IDP = 22,             /* XNS IDP protocol                     */
  IPPROTO_DCCP = 33,            /* Datagram Congestion Control Protocol */
  IPPROTO_RSVP = 46,            /* RSVP protocol                        */
  IPPROTO_GRE = 47,             /* Cisco GRE tunnels (rfc 1701,1702)    */

  IPPROTO_IPV6   = 41,          /* IPv6-in-IPv4 tunnelling              */

  IPPROTO_ESP = 50,            /* Encapsulation Security Payload protocol */
  IPPROTO_AH = 51,             /* Authentication Header protocol       */
  IPPROTO_BEETPH = 94,         /* IP option pseudo header for BEET */
  IPPROTO_PIM    = 103,         /* Protocol Independent Multicast       */

  IPPROTO_COMP   = 108,                /* Compression Header protocol */
  IPPROTO_SCTP   = 132,         /* Stream Control Transport Protocol    */

  IPPROTO_RAW    = 255,         /* Raw IP packets                       */
  IPPROTO_MAX
};

typedef unsigned short sa_family_t;
typedef uint32_t socklen_t;

struct sockaddr {
        sa_family_t     sa_family;      /* address family, AF_xxx       */
        char            sa_data[14];    /* 14 bytes of protocol address */
};

typedef uint32_t in_addr_t;
/* Internet address. */
struct in_addr {
    in_addr_t   s_addr;
};

/* Structure describing an Internet (IP) socket address. */
#define __SOCK_SIZE__   16              /* sizeof(struct sockaddr)      */
struct sockaddr_in {
  sa_family_t           sin_family;     /* Address family               */
  unsigned short int    sin_port;       /* Port number                  */
  struct in_addr        sin_addr;       /* Internet address             */

  /* Pad to size of `struct sockaddr'. */
  unsigned char         __pad[__SOCK_SIZE__ - sizeof(short int) -
                        sizeof(unsigned short int) - sizeof(struct in_addr)];
};

struct hostent {
    char    *h_name;        /* official name of host */
    char    **h_aliases;    /* alias list */
    int     h_addrtype;     /* host address type */
    int     h_length;       /* length of address */
    char    **h_addr_list;  /* list of addresses */
};
#define h_addr  h_addr_list[0]  /* for backward compatibility */

struct servent {
    char    *s_name;        /* official service name */
    char    **s_aliases;    /* alias list */
    int     s_port;         /* port number */
    char    *s_proto;       /* protocol to use */
};

#endif
