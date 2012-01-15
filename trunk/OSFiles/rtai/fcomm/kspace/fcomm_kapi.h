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
#ifndef FCOMM_KAPI_H
#define	FCOMM_KAPI_H

#include "fcomm_kernel_imports.h"
#if !defined(__cplusplus)
#include "rtai_sem.h"
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#define VPRINTK_BUFFER_SIZE 1024

    uint16_t fcomm_htons(uint16_t hostlong);
    uint32_t fcomm_htonl(uint32_t hostlong);
    int open(const char *pathname, int flags, int mode);
    int write(int fd, const void *buf, int count);
    int close(int fd);
    int read(int fd, const void *buf, int count);
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    int dup(int oldfd);
    int dup2(int oldfd, int newfd);
    int pipe(int filedes[2]);
    int socket(int domain, int type, int protocol);
    ssize_t send(int s, const void *buf, size_t len, int flags);
    ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
    ssize_t recv(int s, const void *buf, size_t len, int flags);
    ssize_t recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
    int fcntl(int fd, int cmd, long arg);
    int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
    int listen(int sockfd, int backlog);
    int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
    int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
    int rand(void);
    int srand(unsigned int seed);
    int mkdir(const char *pathname, mode_t mode);
    int rmdir(const char *pathname);
    int unlink(const char *pathname);
    int gethostname(char *name, size_t len);
    int sethostname(const char *name, size_t len);
    int getdomainname(char *name, size_t len);
    int setdomainname(const char *name, size_t len);
    int getpeername(int s, struct sockaddr *name, socklen_t *namelen);
    int fcomm_getservbyname(const char *name, const char *proto, void* serv);
    int fcomm_getservbyport(int port, const char *proto, void* serv);
    in_addr_t inet_addr(const char *cp);
    int inet_aton(const char *cp, struct in_addr *inp);
    in_addr_t inet_network(const char *cp);
    char *inet_ntoa(char *buffer, int buffer_len, struct in_addr in);
    in_addr_t inet_lnaof(struct in_addr in);
    in_addr_t inet_netof(struct in_addr in);
    int fstat(int filedes, void *buf);
    off_t lseek(int fildes, off_t offset, int whence);
    int flock(int fd, int operation);
    int remove(const char *pathname);
    time_t time(time_t *t);
    time_t fast_time(void);
    int getchar(void);
    int opendir(const char *name);
    int readdir(int fd, struct dirent *d);
    int closedir(int fd);
    int stat(const char *path, void *buf);
    int fopen(const char *path, const char *mode);
    int ftruncate(int fd, off_t length);
    size_t fread(void *ptr, size_t size, size_t nmemb, int fd);
    size_t fwrite(const void *ptr, size_t size, size_t nmemb, int fd);
    int fclose(int fd);
    int fseek(int fd, long offset, int whence);
    long ftell(int fd);
    char *fgets(char *s, int size, int fd);
    char *ctime(char* buffer, int buffer_len, const time_t *timep);    
    int ferror(int fd);
    int fcomm_vprintk(const char *format, va_list ap);
    int fcomm_sscanf(const char *buf, const char *fmt, ...);
    int fcomm_snprintf(char *buf, size_t size, const char *fmt, ...);
    int fcomm_vsnprintf(char *str, size_t size, const char *format, va_list ap);
    int fcomm_sprintf(char *buf, const char *fmt, ...);
    int fcomm_strnicmp(const char * s1, const char * s2, size_t len);
    int fcomm_rt_task_init(struct rt_task_struct *task, long data, int stack_size, int priority, int uses_fpu, void(*signal)(void));
    int fcomm_rt_task_init_cpuid(struct rt_task_struct *task, long data, int stack_size, int priority, int uses_fpu, void(*signal)(void), unsigned int cpuid);
    int fcomm_get_errno(void);
    void *fcomm_find_function_by_name(char *name, char *moduleName);
    int fflush(int fd);
    int setblocking(int sock, int flag);
    long *fcomm_get_stack_pointer(void);
    int fcomm_rt_check_current_stack(void);
    int exec_bash(const char* command);
    int fcomm_get_number_of_online_cpus(void);
    int fcomm_set_console_performing_char_input(int);
    int fcomm_get_baselib2_allocated_mem(void);
    int fcomm_get_baselib2_total_mem(void);
    int fcomm_get_fcomm_allocated_mem(void);
    int fcomm_get_fcomm_total_mem(void);
    int fcomm_get_large_heap_allocated_mem(void);
    int fcomm_get_large_heap_total_mem(void);
    int fcomm_get_internetinfo(char* hostname, char* ipaddr, int size);
    int fcomm_gethostbyname(const char* hostname);
    void fcomm_gethostbyaddr(const void* addr, char* buffer, int buffer_len);
    unsigned long int fcomm_get_max_block_size_baselib2(void);
    unsigned long int fcomm_get_max_block_size_large_heap(void);
    unsigned long int fcomm_get_max_block_size_fcomm(void);
#if defined(__cplusplus)    
    void fcomm_print_sem(struct SEM *sem);
#else
    unsigned long int fcomm_get_max_block_size(rtheap_t *heap);
    void fcomm_print_sem(SEM *sem);
#endif
#if defined(__cplusplus)
}
#endif

#endif
