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
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <termio.h>

#include <rtai_shm.h>
#include "../share/fcomm_share.h"
#include "api.h"
#include "fcomm_ipc.h"
#include "hashtable.h"

//The original shell modes
static struct termio original_shell_modes;

static FILE *fifo_read = 0;
static FILE *fifo_write = 0;

void clear_screen(void)
{
    int i = 0;
    for(i = 0; i < 64; i++)
        printf("\n");
}

static struct hashtable *dir_hash;
static struct hashtable *file_hash;
typedef struct fd_struct
{
    int fd;
}fd_struct;

typedef struct desc_pointer
{
    void *ptr;
}desc_pointer;

static unsigned int hash_from_key_fn( void *k )
{
    return ((fd_struct *)k)->fd;
}

static int keys_equal_fn ( void *key1, void *key2 )
{
    return (((fd_struct *)key1)->fd == ((fd_struct *)key2)->fd);
}

int init_hashtables(void)
{
    dir_hash = create_hashtable(16, hash_from_key_fn, keys_equal_fn);
    file_hash = create_hashtable(16, hash_from_key_fn, keys_equal_fn);
    return (dir_hash == NULL || file_hash == NULL) ? -1 : 0;
}

void destroy_hashtables(void)
{
    hashtable_destroy(dir_hash, 1);
    hashtable_destroy(file_hash, 1);
}

int init_api(void){
    signal (SIGPIPE, SIG_IGN);
    init_gen_function_table();
#ifdef _USE_CONSOLE_FIFOS_    
    fifo_read = fopen(RTAI_FIFO_INPUT, "r+");
    fifo_write = fopen(RTAI_FIFO_OUTPUT, "r+");
    if(fifo_read == NULL || fifo_write == NULL){
        printf("COULD NOT OPEN RTAI CONSOLE FIFOS!\n");
    }
#endif    
    return init_hashtables();
}

void close_api(void){
    destroy_hashtables();
#ifdef _USE_CONSOLE_FIFOS_        
    fclose(fifo_read);
    fclose(fifo_write);
#endif    
}

static void *check_null(unsigned long id)
{
    return (id == NULL_PARAMETER_ID ? NULL : (void*)rt_named_malloc(id, 0));
}

static int fcomm_open(mem_function *mem)
{    
    const char *pathname = (const char*)check_null(mem->parameter_ids[0]);
    int flags = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    mode_t mode = *((mode_t *)rt_named_malloc(mem->parameter_ids[2], 0));
    char message[256];
    int ret = 0;
    ret = open(pathname, flags, mode);
    if(loggerVerboseLevel > VERBOSE_ALL){
        sprintf(message, "Going to open file %s with flags=%d and mode=%d", pathname, flags, mode);
        logUDPStandardMessage(1, message);    
        memset(message, 0, strlen(message));
        sprintf(message, "Open returned %d", ret);
        logUDPStandardMessage(0, message);
        if(ret == -1){
            memset(message, 0, strlen(message));
            sprintf(message, "Error = %s", strerror(errno));
            logUDPStandardMessage(-1, message);
        }
    }
    return ret;
}

static int fcomm_write(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const void *buf = (const void*)check_null(mem->parameter_ids[1]);
    size_t count = *((size_t *)rt_named_malloc(mem->parameter_ids[2], 0));
#ifdef _USE_CONSOLE_FIFOS_    
    if(fd == STDOUT_FILENO){
        fd = fileno(fifo_write);
    }
#endif
    return write(fd, buf, count);
}

static int fcomm_close(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    return close(fd);                             
}

static int fcomm_read(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    void *buf = (void*)check_null(mem->parameter_ids[1]);
    size_t count = *((size_t *)rt_named_malloc(mem->parameter_ids[2], 0));
#ifdef _USE_CONSOLE_FIFOS_    
    if(fd == STDIN_FILENO){
        fd = fileno(fifo_read);
    }
#endif    
    return read(fd, buf, count);
}

static int fcomm_fstat(mem_function *mem)
{
    int filedes = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    struct stat *buf = (struct stat *)check_null(mem->parameter_ids[1]);
    int ret = fstat(filedes, buf);    
    if(loggerVerboseLevel > VERBOSE_ALL){
        char message[256];
        sprintf(message, "fstat returned %d and buf->st_mode = %d\n", ret, buf->st_mode);    
        logUDPStandardMessage(0, message);
        if(ret == -1){
            memset(message, 0, strlen(message));
            sprintf(message, "Error = %s", strerror(errno));
            logUDPStandardMessage(-1, message);
        }
    }
    return ret;
}

static int fcomm_lseek(mem_function *mem)
{
    int fildes = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    off_t offset = *((off_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    int whence = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    off_t *ret = (off_t *)rt_named_malloc(mem->parameter_ids[3], 0);
    *ret = lseek(fildes, offset, whence);
    return 0;
}

static int fcomm_flock(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int operation = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    return flock(fd, operation);
}

static int fcomm_select(mem_function *mem)
{
    int nfds = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    fd_set *readfds = (fd_set *)check_null(mem->parameter_ids[1]);
    fd_set *writefds = (fd_set *)check_null(mem->parameter_ids[2]);
    fd_set *exceptfds = (fd_set *)check_null(mem->parameter_ids[3]);
    struct timeval *timeout = (struct timeval *)check_null(mem->parameter_ids[4]);
    return select(nfds, readfds, writefds, exceptfds, timeout);        
}

static int fcomm_dup(mem_function *mem)
{
    int oldfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    return dup(oldfd);                             
}

static int fcomm_dup2(mem_function *mem)
{
    int oldfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int newfd = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    return dup2(oldfd, newfd); 
}

static int fcomm_pipe(mem_function *mem)
{
    int *filedes = (int *)check_null(mem->parameter_ids[0]);
    return pipe(filedes);
}

static int fcomm_socket(mem_function *mem)
{
    int domain = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int type = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    int protocol = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));    
    return socket(domain, type, protocol);
}

static int  fcomm_send(mem_function *mem)
{
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const void *buf = (const void *)check_null(mem->parameter_ids[1]);
    size_t len = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    size_t flags = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));
    //We dont want to die with this signal, whatever happens!
    flags |= MSG_NOSIGNAL;
    return send(s, buf, len, flags);    
}

static int  fcomm_sendto(mem_function *mem)
{
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const void *buf = (const void *)check_null(mem->parameter_ids[1]);
    size_t len = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    size_t flags = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));
    //We dont want to die with this signal, whatever happens!
    flags |= MSG_NOSIGNAL;
    const struct sockaddr *to = (const struct sockaddr *)check_null(mem->parameter_ids[4]);
    socklen_t tolen = *((socklen_t *)rt_named_malloc(mem->parameter_ids[5], 0));
    return sendto(s, buf, len, flags, to, tolen);    
}

static int  fcomm_recv(mem_function *mem)
{
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const void *buf = (const void *)check_null(mem->parameter_ids[1]);
    size_t len = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    size_t flags = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));
    return recv(s, (void *)buf, len, flags);    
}

static int  fcomm_recvfrom(mem_function *mem)
{
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const void *buf = (const void *)check_null(mem->parameter_ids[1]);
    size_t len = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    size_t flags = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));
    struct sockaddr *to = (struct sockaddr *)check_null(mem->parameter_ids[4]);
    socklen_t *tolen = (socklen_t *)rt_named_malloc(mem->parameter_ids[5], 0);
    return recvfrom(s, (void *)buf, len, flags, to, tolen);    
}

static int fcomm_fcntl(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int cmd = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    long opts = *((long *)rt_named_malloc(mem->parameter_ids[2], 0));
    return fcntl(fd, cmd, opts);
}

static int fcomm_bind(mem_function *mem)
{
    int sockfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    void *my_addr = (void *)check_null(mem->parameter_ids[1]);
    socklen_t addrlen= *((socklen_t *)rt_named_malloc(mem->parameter_ids[2], 0));    
    return bind(sockfd, my_addr, addrlen);    
}

int fcomm_connect(mem_function *mem)
{
    int sockfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    const struct sockaddr *serv_addr = (const struct sockaddr *)check_null(mem->parameter_ids[1]);
    socklen_t addrlen= *((socklen_t *)rt_named_malloc(mem->parameter_ids[2], 0));    
    return connect(sockfd, serv_addr, addrlen);
}

static int fcomm_listen(mem_function *mem)
{
    int sockfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int backlog = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    return listen(sockfd, backlog);
}

static int fcomm_accept(mem_function *mem)
{
    int sockfd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    void *addr = (void *)check_null(mem->parameter_ids[1]);
    socklen_t *addrlen= (socklen_t *)rt_named_malloc(mem->parameter_ids[2], 0);
    return accept(sockfd, addr, addrlen);
}

static int fcomm_getsockopt(mem_function *mem)
{    
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int level = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    int optname = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    void *optval = (void *)check_null(mem->parameter_ids[3]);
    socklen_t *optlen= (socklen_t *)check_null(mem->parameter_ids[4]);
    return getsockopt(s, level, optname, optval, optlen);
}

static int fcomm_setsockopt(mem_function *mem)
{    
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int level = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    int optname = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    void *optval = (void *)check_null(mem->parameter_ids[3]);
    socklen_t optlen= *((socklen_t *)rt_named_malloc(mem->parameter_ids[4], 0));
    return setsockopt(s, level, optname, optval, optlen);
}

static int fcomm_srand(mem_function *mem)
{    
    int seed = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    srand(seed);
    return 0;
}

static int fcomm_rand(mem_function *mem)
{        
    return rand();
}

static int fcomm_mkdir(mem_function *mem)
{        
    void *pathname = (void *)check_null(mem->parameter_ids[0]);
    mode_t mode= *((mode_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return mkdir(pathname, mode);
}

static int fcomm_rmdir(mem_function *mem)
{        
    void *pathname = (void *)check_null(mem->parameter_ids[0]);    
    return rmdir(pathname);
}

static int fcomm_unlink(mem_function *mem)
{        
    void *pathname = (void *)check_null(mem->parameter_ids[0]);    
    return unlink(pathname);
}

static int fcomm_remove(mem_function *mem)
{        
    void *pathname = (void *)check_null(mem->parameter_ids[0]);    
    return remove(pathname);
}

static int fcomm_gethostname(mem_function *mem)
{
    void *name = (void *)check_null(mem->parameter_ids[0]);
    size_t len= *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return gethostname(name, len);
}

static int fcomm_sethostname(mem_function *mem)
{
    void *name = (void *)check_null(mem->parameter_ids[0]);
    size_t len= *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return sethostname(name, len);
}

static int fcomm_getdomainname(mem_function *mem)
{
    void *name = (void *)check_null(mem->parameter_ids[0]);
    size_t len= *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return getdomainname(name, len);
}

static int fcomm_setdomainname(mem_function *mem)
{
    void *name = (void *)check_null(mem->parameter_ids[0]);
    size_t len= *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return setdomainname(name, len);
}

static int fcomm_getservbyname(mem_function *mem)
{
    char *name = (char *)check_null(mem->parameter_ids[0]);
    char *proto = (char *)check_null(mem->parameter_ids[1]);
    struct servent *serv = (struct servent*)check_null(mem->parameter_ids[2]);

    struct servent *temp = (struct servent *)getservbyname(name, proto);
    
    if (temp==NULL) return -1;
    else {
        memcpy(serv,temp,sizeof(struct servent));
    }
    free(temp);
    return 0;
}

static int fcomm_getservbyport(mem_function *mem)
{
    int port = *((int *)check_null(mem->parameter_ids[0]));
    char *proto = (char *)check_null(mem->parameter_ids[1]);
    struct servent *serv = (struct servent*)check_null(mem->parameter_ids[2]);

    struct servent *temp = (struct servent *)getservbyport(port, proto);

    if (temp==NULL) return -1;
    else {
        memcpy(serv,temp,sizeof(struct servent));
    }
    free(temp);
    return 0;
}

static int fcomm_inet_addr(mem_function *mem)
{
    char *cp = (char *)check_null(mem->parameter_ids[0]);
    *((in_addr_t *)rt_named_malloc(mem->parameter_ids[1], 0)) = inet_addr(cp);
    return 0;
}

static int fcomm_inet_aton(mem_function *mem)
{
    char *cp = (char *)check_null(mem->parameter_ids[0]);
    struct in_addr *inp = (struct in_addr *)check_null(mem->parameter_ids[1]);
    return inet_aton(cp, inp);
}

int fcomm_inet_network(mem_function *mem)
{
    char *cp = (char *)check_null(mem->parameter_ids[0]);
    in_addr_t ret = inet_network(cp);
    *((in_addr_t *)rt_named_malloc(mem->parameter_ids[1], 0)) = ret;
    return 0;
}

int fcomm_inet_ntoa(mem_function *mem)
{
    char  *buffer      = (char *)check_null(mem->parameter_ids[0]);
    int    buffer_size = *(int*)rt_named_malloc(mem->parameter_ids[1], 0);
    struct in_addr *in = (struct in_addr *)check_null(mem->parameter_ids[2]);
    char *ret;
    if(buffer == NULL || in == NULL){
        return -1;
    }
    ret = inet_ntoa(*in);
    if(ret == NULL){
        return -1;
    }
    else{
        strncpy(buffer, ret, buffer_size);
    }
    
    return 0;
}

int fcomm_inet_lnaof(mem_function *mem)
{
    struct in_addr *in = (struct in_addr *)check_null(mem->parameter_ids[0]);
    in_addr_t ret = inet_lnaof(*in);
    *((in_addr_t *)rt_named_malloc(mem->parameter_ids[1], 0)) = ret;
    return 0;
}

int fcomm_inet_netof(mem_function *mem)
{
    struct in_addr *in = (struct in_addr *)check_null(mem->parameter_ids[0]);
    in_addr_t ret = inet_netof(*in);
    *((in_addr_t *)rt_named_malloc(mem->parameter_ids[1], 0)) = ret;
    return 0;
}

int fcomm_getpeername(mem_function *mem)
{
    int s = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    struct sockaddr *name = (struct sockaddr *)check_null(mem->parameter_ids[1]);
    socklen_t *namelen = (socklen_t *)check_null(mem->parameter_ids[2]);
    
    return getpeername(s, name, namelen);
}

int fcomm_time(mem_function *mem)
{
    time_t *t = (time_t *)check_null(mem->parameter_ids[0]);
    *((time_t *)rt_named_malloc(mem->parameter_ids[1], 0)) = time(t);
    
    return 0;
}

int fcomm_getchar(mem_function *mem)
{
#ifdef _USE_CONSOLE_FIFOS_
    FILE *stream = fifo_read;
#else
    FILE *stream = stdin;
#endif
    return getc(stream);    
}

int fcomm_fgets(mem_function *mem)
{
    char *s = (char *)check_null(mem->parameter_ids[0]);
    int size = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    FILE *stream = NULL;
    char *ret = NULL;
    if(fd == STDIN_FILENO){
#ifdef _USE_CONSOLE_FIFOS_    
    if(fd == STDIN_FILENO){
        stream = fifo_read;
    }
#else
        stream = stdin;
#endif
    }
    else
    {
        value = (desc_pointer *)hashtable_search(file_hash, &key);
        if(value == NULL)
            return NULL_PARAMETER_ID;            
        stream = value->ptr;
    }
    
    ret = fgets(s, size, stream);
#ifdef _USE_CONSOLE_FIFOS_
    if(size > 1){
	s[1] = 0;
    }
#endif
    return ret == NULL ? NULL_PARAMETER_ID : 0;
}

static int fcomm_fflush(mem_function *mem)
{
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    FILE *stream = NULL;
    
    if(fd == 0)
#ifdef _USE_CONSOLE_FIFOS_
	stream = fifo_read;
#else
        stream = stdin;
#endif
    else if(fd == 1)
        stream = stdout;
    else if(fd == 2)
        stream = stderr;
    else
    {
        value = (desc_pointer *)hashtable_search(file_hash, &key);
        if(value == NULL)
            return NULL_PARAMETER_ID;
        stream = value->ptr;
    }
    
    return fflush(stream);
}

static int fcomm_closedir(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(dir_hash, &key);    
    
    if(value != NULL)
    {
        ret = closedir((DIR *)value->ptr);
        hashtable_remove(dir_hash, &key);
        free(value);
    }
    
    return ret;
}

static int fcomm_readdir(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    struct dirent *to_copy = (struct dirent *)check_null(mem->parameter_ids[1]);
    struct dirent *dir = NULL;
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(dir_hash, &key);    
    
    if(value != NULL)
    {
        dir = readdir((DIR *)value->ptr);
        if(dir != NULL)
        {
            memcpy(to_copy, dir, sizeof(struct dirent));
            ret = 1;
        }
    }
    
    return ret;
}

static int fcomm_opendir(mem_function *mem)
{    
    const char *name = (const char*)check_null(mem->parameter_ids[0]);
    DIR *d = opendir(name);
    int ret = 0;
    fd_struct *key;
    desc_pointer *value;
    
    if(d == NULL)
        return NULL_PARAMETER_ID;
    
    ret = dirfd(d);
    key = (fd_struct *)malloc(sizeof(fd_struct));
    key->fd = ret;
    value = (desc_pointer *)malloc(sizeof(desc_pointer));
    value->ptr = d;
    hashtable_insert(dir_hash, key, value);
    
    return ret;
}

static int fcomm_stat(mem_function *mem)
{
    const char *path = (const char *)check_null(mem->parameter_ids[0]);
    struct stat *buf = (struct stat *)check_null(mem->parameter_ids[1]);    
    return stat(path, buf);
}

static int fcomm_ftruncate(mem_function *mem){
    int   fd     = *((int *)rt_named_malloc(mem->parameter_ids[0], 0)); 
    off_t length = *((off_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    return ftruncate(fd, length);
}


static int fcomm_fopen(mem_function *mem)
{    
    const char *path = (const char*)check_null(mem->parameter_ids[0]);
    const char *mode = (const char*)check_null(mem->parameter_ids[1]);
    FILE *f = fopen(path, mode);
    int ret = 0;
    fd_struct *key;
    desc_pointer *value;
    
    if(f == NULL)
        return NULL_PARAMETER_ID;
    
    ret = fileno(f);
    key = (fd_struct *)malloc(sizeof(fd_struct));
    key->fd = ret;
    value = (desc_pointer *)malloc(sizeof(desc_pointer));
    value->ptr = f;
    hashtable_insert(file_hash, key, value);
    
    return ret;
}

int register_file_descriptor(int fildes, const char *mode)
{
    fd_struct *key;
    desc_pointer *value;
    FILE *f = fdopen(fildes, mode);    
    if(f == NULL)
        return -1;
    
    key = (fd_struct *)malloc(sizeof(fd_struct));
    key->fd = fildes;
    value = (desc_pointer *)malloc(sizeof(desc_pointer));
    value->ptr = f;
    hashtable_insert(file_hash, key, value);   
    
    return 0;
}

static int fcomm_fread(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;    
    void *ptr = check_null(mem->parameter_ids[0]);
    size_t size = *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    size_t nmemb = *((size_t *)rt_named_malloc(mem->parameter_ids[2], 0));
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));    
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            ret = fread(ptr, size, nmemb, f);            
        }
    }
    
    return ret;
}

static int fcomm_fwrite(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;    
    const void *ptr = (const void *)check_null(mem->parameter_ids[0]);
    size_t size = *((size_t *)rt_named_malloc(mem->parameter_ids[1], 0));
    size_t nmemb = *((size_t *)rt_named_malloc(mem->parameter_ids[2], 0));
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[3], 0));    
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            ret = fwrite(ptr, size, nmemb, f);            
        }
    }
    
    return ret;
}

static int fcomm_fseek(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;    
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    long offset = *((long *)rt_named_malloc(mem->parameter_ids[1], 0));
    int whence = *((int *)rt_named_malloc(mem->parameter_ids[2], 0));
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            ret = fseek(f, offset, whence);            
        }
    }
    
    return ret;
}

static int fcomm_fclose(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;    
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            ret = fclose(f);
            hashtable_remove(file_hash, &key);
            free(value);
        }
    }
    
    return ret;
}

static int fcomm_ftell(mem_function *mem)
{    
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    long *to_ret = (long *)rt_named_malloc(mem->parameter_ids[1], 0);
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            *to_ret = ftell(f);
        }
    }
    
    return 0;
}

static int fcomm_ferror(mem_function *mem)
{    
    int ret = NULL_PARAMETER_ID;    
    int fd = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    FILE *f = NULL;
    
    fd_struct key;
    desc_pointer *value = NULL;
    key.fd = fd;
    value = (desc_pointer *)hashtable_search(file_hash, &key);    
    
    if(value != NULL)
    {
        f = (FILE *)value->ptr;
        if(f != NULL)
        {
            ret = ferror(f);            
        }
    }
    
    return ret;
}

static int fcomm_ctime(mem_function *mem)
{    
    char         *buffer      = (char *)check_null(mem->parameter_ids[0]);
    int           buffer_size = *(int*)rt_named_malloc(mem->parameter_ids[1], 0);
    const time_t *timep       = (const time_t *)check_null(mem->parameter_ids[2]);

    if(buffer == NULL){
        return -1;
    }
    char *res = ctime(timep);
    
    if(res == NULL){
        return -1;
    }
    else{
        strncpy(buffer, res, buffer_size);
    }
    
    return 0;
}

static int fcomm_find_func_by_name(mem_function *mem)
{
    char *name = (char *)check_null(mem->parameter_ids[0]);
    char *module_name = (char *)check_null(mem->parameter_ids[1]);
    unsigned long *ptr = (unsigned long *)rt_named_malloc(mem->parameter_ids[2], 0);
    
    FILE *kallsyms = fopen(KALL_SYMS_LOC, "r");
    char *line = NULL;
    ssize_t read;
    size_t len;
    unsigned long address = -1;
    char *tok = NULL;
    
    *ptr = NULL_PARAMETER_ID;
    
    if(kallsyms != NULL)
    {    
        
        if(module_name == NULL)
        {
            while ((read = getline(&line, &len, kallsyms)) != -1) 
            {
                if((strstr(line, name) != NULL))             
                {              
                    tok = strsep(&line, " ");
                    address = strtoll(tok, NULL, 16);
                    strsep(&line, " ");
                    tok = strsep(&line, " ");
                    if((strncmp(tok, name, strlen(name)) == 0) && tok[strlen(name)] == 9)
                    {
                        *ptr = address;
                        break;
                    }
                }
            }
        }
        else
        {
            while ((read = getline(&line, &len, kallsyms)) != -1) 
            {
                if((strstr(line, name) != NULL) && (strstr(line, module_name) != NULL))
                {              
                    tok = strsep(&line, " ");
                    address = strtoll(tok, NULL, 16);
                    strsep(&line, " ");
                    tok = strsep(&line, " ");
                    if((strncmp(tok, name, strlen(name)) == 0) && tok[strlen(name)] == 9)
                    {
                        *ptr = address;
                        break;
                    }
                }
            }
        }
        if (line)
            free(line);

        fclose(kallsyms);
    }
        
    return 0;
}

static int fcomm_setblocking(mem_function *mem)
{
    int stat;
    int socket = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
    int flag = *((int *)rt_named_malloc(mem->parameter_ids[1], 0));
    if (flag == 1) 
        stat = 0;
    else 
        stat = 1;
    return ioctl(socket, FIONBIO, (char *)&stat, sizeof(stat));    
}



static int fcomm_exec_bash(mem_function *mem)
{
    const char *command = (const char *)check_null(mem->parameter_ids[0]);
    int pid = 0;
    if(command == NULL)
        return -1;
    pid = fork();
    if(pid == 0)
    {
        execl("/bin/sh", "sh", "-c", command, NULL);
        return 0;
    }
    return 0;
}

static int fcomm_set_character_input(mem_function *mem){
    int set = *((int *)rt_named_malloc(mem->parameter_ids[0], 0));
        
    struct termio to_modify_modes;
    if(ioctl(fileno(stdin), TCGETA, &original_shell_modes) < 0)
        return 0;
    to_modify_modes = original_shell_modes;
    to_modify_modes.c_lflag &= ~ICANON;
    to_modify_modes.c_cc[VMIN] = 1;
    to_modify_modes.c_cc[VTIME] = 0;
    if(set == 1){
        ioctl(fileno(stdin), TCSETAW, &to_modify_modes);
    }
    
    return 1;
}

static int fcomm_get_internetinfo(mem_function *mem){
    char* hostname = (char *)check_null(mem->parameter_ids[0]);
    char* ipaddr = (char *)check_null(mem->parameter_ids[1]);
    int size = *(int*)rt_named_malloc(mem->parameter_ids[2], 0);

    if ((ipaddr!=NULL) && (hostname!=NULL)) {
        struct hostent *h = NULL;
        int ret = gethostname(hostname,size);
	if (ret != 0) return 1;
	h = gethostbyname(hostname);
	if (h == NULL) return 1;
	strcpy(hostname,h->h_name);
	struct in_addr sin_addr;
	sin_addr.s_addr = *((int *)(h->h_addr_list[0]));
        strcpy(ipaddr,inet_ntoa(sin_addr));

	return 0;
    }

    return 1;
}

static int fcomm_gethostbyname(mem_function *mem){
    char* hostname = (char *)check_null(mem->parameter_ids[0]);

    if (hostname!=NULL) {
        struct hostent *h = gethostbyname(hostname);

        if (h==NULL) return 0;
        return *((int*)(h->h_addr_list[0]));
    }
    return 0;
}

static int fcomm_gethostbyaddr(mem_function *mem){
    void* addr = (void *)check_null(mem->parameter_ids[0]);
    char* buffer = (char *)check_null(mem->parameter_ids[1]);
    int buffer_len = *(int*)rt_named_malloc(mem->parameter_ids[2], 0);

    struct hostent *h = gethostbyaddr(addr,4,AF_INET);

    if (h!=NULL) {
        strncpy(buffer,h->h_name,buffer_len);
    } else {
        strcpy(buffer,"unknown");
    }

    return 0;
}

void init_gen_function_table(void)
{
    gen_function_table[FUN_ID_OPEN] = &fcomm_open;
    gen_function_table[FUN_ID_WRITE] = &fcomm_write;
    gen_function_table[FUN_ID_CLOSE] = &fcomm_close;
    gen_function_table[FUN_ID_READ] = &fcomm_read;
    gen_function_table[FUN_ID_SELECT] = &fcomm_select;
    gen_function_table[FUN_ID_DUP] = &fcomm_dup;
    gen_function_table[FUN_ID_DUP_2] = &fcomm_dup2;
    gen_function_table[FUN_ID_PIPE] = &fcomm_pipe;
    gen_function_table[FUN_ID_SOCKET] = &fcomm_socket;
    gen_function_table[FUN_ID_SEND] = &fcomm_send;
    gen_function_table[FUN_ID_SEND_TO] = &fcomm_sendto;
    gen_function_table[FUN_ID_RECV] = &fcomm_recv;
    gen_function_table[FUN_ID_RECV_FROM] = &fcomm_recvfrom;
    gen_function_table[FUN_ID_FCNTL] = &fcomm_fcntl;    
    gen_function_table[FUN_ID_BIND] = &fcomm_bind;
    gen_function_table[FUN_ID_LISTEN] = &fcomm_listen;
    gen_function_table[FUN_ID_ACCEPT] = &fcomm_accept;
    gen_function_table[FUN_ID_GET_SOCK_OPT] = &fcomm_getsockopt;
    gen_function_table[FUN_ID_SET_SOCK_OPT] = &fcomm_setsockopt;
    gen_function_table[FUN_ID_RAND] = &fcomm_rand;
    gen_function_table[FUN_ID_SRAND] = &fcomm_srand;
    gen_function_table[FUN_ID_MKDIR] = &fcomm_mkdir;
    gen_function_table[FUN_ID_RMDIR] = &fcomm_rmdir;
    gen_function_table[FUN_ID_UNLINK] = &fcomm_unlink;
    gen_function_table[FUN_ID_GET_HOSTNAME] = &fcomm_gethostname;
    gen_function_table[FUN_ID_SET_HOSTNAME] = &fcomm_sethostname;
    gen_function_table[FUN_ID_GET_DOMAINNAME] = &fcomm_getdomainname;
    gen_function_table[FUN_ID_SET_DOMAINNAME] = &fcomm_setdomainname;
    gen_function_table[FUN_ID_FCOMM_GETSERVBYNAME] = &fcomm_getservbyname;
    gen_function_table[FUN_ID_FCOMM_GETSERVBYPORT] = &fcomm_getservbyport;
    gen_function_table[FUN_ID_INET_ADDR] = &fcomm_inet_addr;
    gen_function_table[FUN_ID_INET_ATON] = &fcomm_inet_aton;
    gen_function_table[FUN_ID_INET_NETWORK] = &fcomm_inet_network;
    gen_function_table[FUN_ID_INET_NTOA] = &fcomm_inet_ntoa;
    gen_function_table[FUN_ID_INET_LNAOF] = &fcomm_inet_lnaof;
    gen_function_table[FUN_ID_INET_NETOF] = &fcomm_inet_netof;
    gen_function_table[FUN_ID_FSTAT] = &fcomm_fstat;
    gen_function_table[FUN_ID_LSEEK] = &fcomm_lseek;
    gen_function_table[FUN_ID_FLOCK] = &fcomm_flock;
    gen_function_table[FUN_ID_REMOVE] = &fcomm_remove;
    gen_function_table[FUN_ID_GET_PEERNAME] = &fcomm_getpeername;
    gen_function_table[FUN_ID_TIME] = &fcomm_time;
    gen_function_table[FUN_ID_CONNECT] = &fcomm_connect;
    gen_function_table[FUN_ID_GETCHAR] = &fcomm_getchar;
    gen_function_table[FUN_ID_FGETS] = &fcomm_fgets;
    gen_function_table[FUN_ID_FFLUSH] = &fcomm_fflush;
    gen_function_table[FUN_ID_OPENDIR] = &fcomm_opendir;
    gen_function_table[FUN_ID_CLOSEDIR] = &fcomm_closedir;
    gen_function_table[FUN_ID_READDIR] = &fcomm_readdir;
    gen_function_table[FUN_ID_STAT] = &fcomm_stat;
    gen_function_table[FUN_ID_FOPEN] = &fcomm_fopen;
    gen_function_table[FUN_ID_FREAD] = &fcomm_fread;
    gen_function_table[FUN_ID_FWRITE] = &fcomm_fwrite;
    gen_function_table[FUN_ID_FCLOSE] = &fcomm_fclose;
    gen_function_table[FUN_ID_FSEEK] = &fcomm_fseek;
    gen_function_table[FUN_ID_FTELL] = &fcomm_ftell;
    gen_function_table[FUN_ID_CTIME] = &fcomm_ctime;
    gen_function_table[FUN_ID_FERROR] = &fcomm_ferror;
    gen_function_table[FUN_ID_FIND_FUNC_BY_NAME] = &fcomm_find_func_by_name;
    gen_function_table[FUN_ID_SET_SOCKET_BLOCKING] = &fcomm_setblocking;
    gen_function_table[FUN_ID_EXEC_BASH] = &fcomm_exec_bash;
    gen_function_table[FUN_ID_CON_PERFORM_CHAR_INPUT] = &fcomm_set_character_input;    
    gen_function_table[FUN_ID_GET_INTERNETINFO] = &fcomm_get_internetinfo;    
    gen_function_table[FUN_ID_FCOMM_GETHOSTBYNAME] = &fcomm_gethostbyname;    
    gen_function_table[FUN_ID_FCOMM_GETHOSTBYADDR] = &fcomm_gethostbyaddr;    
    gen_function_table[FUN_ID_FTRUNCATE] = &fcomm_ftruncate;
}
