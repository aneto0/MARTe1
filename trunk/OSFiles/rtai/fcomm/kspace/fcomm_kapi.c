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

#include <rtai_shm.h>
#include <rtai_sched.h>
#include <rtai_malloc.h>
#include <rtai_nam2num.h>
#include <rtai_usi.h>
#include <rtai_schedcore.h>

#include "fcomm_kapi.h"
#include "fcomm_mod.h"
#include "../share/fcomm_share.h"
#include "../../../../Level0/SystemRTAI.h"
#include <linux/version.h>

EXPORT_SYMBOL(open);
EXPORT_SYMBOL(write);
EXPORT_SYMBOL(close);
EXPORT_SYMBOL(read);
EXPORT_SYMBOL(fstat);
EXPORT_SYMBOL(stat);
EXPORT_SYMBOL(flock);
EXPORT_SYMBOL(lseek);
EXPORT_SYMBOL(remove);
EXPORT_SYMBOL(select);
EXPORT_SYMBOL(dup);
EXPORT_SYMBOL(dup2);
EXPORT_SYMBOL(pipe);
EXPORT_SYMBOL(socket);
EXPORT_SYMBOL(send);
EXPORT_SYMBOL(sendto);
EXPORT_SYMBOL(recv);
EXPORT_SYMBOL(recvfrom);
EXPORT_SYMBOL(fcntl);
EXPORT_SYMBOL(bind);
EXPORT_SYMBOL(connect);
EXPORT_SYMBOL(listen);
EXPORT_SYMBOL(accept);
EXPORT_SYMBOL(getsockopt);
EXPORT_SYMBOL(setsockopt);
EXPORT_SYMBOL(rand);
EXPORT_SYMBOL(srand);
EXPORT_SYMBOL(mkdir);
EXPORT_SYMBOL(rmdir);
EXPORT_SYMBOL(unlink);
EXPORT_SYMBOL(gethostname);
EXPORT_SYMBOL(sethostname);
EXPORT_SYMBOL(getpeername);
EXPORT_SYMBOL(inet_addr);
EXPORT_SYMBOL(inet_aton);
EXPORT_SYMBOL(inet_network);
EXPORT_SYMBOL(inet_ntoa);
EXPORT_SYMBOL(inet_lnaof);
EXPORT_SYMBOL(inet_netof);
EXPORT_SYMBOL(time);
EXPORT_SYMBOL(getchar);
EXPORT_SYMBOL(fgets);
EXPORT_SYMBOL(fflush);
EXPORT_SYMBOL(opendir);
EXPORT_SYMBOL(readdir);
EXPORT_SYMBOL(closedir);
EXPORT_SYMBOL(fopen);
EXPORT_SYMBOL(ftruncate);
EXPORT_SYMBOL(fread);
EXPORT_SYMBOL(fwrite);
EXPORT_SYMBOL(fclose);
EXPORT_SYMBOL(fseek);
EXPORT_SYMBOL(ftell);
EXPORT_SYMBOL(ferror);
EXPORT_SYMBOL(ctime);
EXPORT_SYMBOL(fcomm_vprintk);
EXPORT_SYMBOL(fcomm_sscanf);
EXPORT_SYMBOL(fcomm_snprintf);
EXPORT_SYMBOL(fcomm_vsnprintf);
EXPORT_SYMBOL(fcomm_sprintf);
EXPORT_SYMBOL(fcomm_strnicmp);
EXPORT_SYMBOL(fcomm_htons);
EXPORT_SYMBOL(fcomm_htonl);
EXPORT_SYMBOL(fcomm_rt_task_init);
EXPORT_SYMBOL(fcomm_rt_task_init_cpuid);
EXPORT_SYMBOL(fcomm_get_errno);
EXPORT_SYMBOL(fcomm_find_function_by_name);
EXPORT_SYMBOL(setblocking);
EXPORT_SYMBOL(fcomm_get_stack_pointer);
EXPORT_SYMBOL(fcomm_print_sem);
EXPORT_SYMBOL(fcomm_rt_check_current_stack);
EXPORT_SYMBOL(fcomm_get_number_of_online_cpus);
EXPORT_SYMBOL(fcomm_set_console_performing_char_input);
EXPORT_SYMBOL(fcomm_get_internetinfo);
EXPORT_SYMBOL(fcomm_gethostbyname);
EXPORT_SYMBOL(fcomm_gethostbyaddr);

/* Exported functions for memory information */
extern rtheap_t baselib_heap;
extern rtheap_t large_heap;
extern rtheap_t rtai_global_heap;

int fcomm_get_baselib2_allocated_mem(void) {
    return baselib_heap.ubytes;
}

int fcomm_get_baselib2_total_mem(void) {
    return baselib_heap.maxcont;
}

int fcomm_get_large_heap_allocated_mem(void) {
    return large_heap.ubytes;
}

int fcomm_get_large_heap_total_mem(void) {
    return large_heap.maxcont;
}

int fcomm_get_fcomm_allocated_mem(void) {
    return rtai_global_heap.ubytes;
}

int fcomm_get_fcomm_total_mem(void) {
    return rtai_global_heap.maxcont;
}

EXPORT_SYMBOL(fcomm_get_baselib2_allocated_mem);
EXPORT_SYMBOL(fcomm_get_baselib2_total_mem);
EXPORT_SYMBOL(fcomm_get_fcomm_allocated_mem);
EXPORT_SYMBOL(fcomm_get_fcomm_total_mem);
EXPORT_SYMBOL(fcomm_get_large_heap_allocated_mem);
EXPORT_SYMBOL(fcomm_get_large_heap_total_mem);
///////////////////////////////

unsigned long int fcomm_get_max_block_size(rtheap_t *heap){
    u_long freecont = 0;
    u_long maxmem = 0;
    struct list_head *holder;
    rtextent_t *extent;
    int pagenum=0;
    list_for_each(holder,&heap->extents) {
        extent = list_entry(holder,rtextent_t,link);
        for(pagenum=0; pagenum < heap->npages; pagenum++){
            if(extent->pagemap[pagenum] == RTHEAP_PFREE){
                freecont += heap->pagesize;
            }
	    else{
                if(freecont > maxmem){
                    maxmem = freecont;
		}
		freecont = 0;
            }
	}
	if(freecont > maxmem){
	    maxmem = freecont;
	}
    }
    return maxmem;
}
EXPORT_SYMBOL(fcomm_get_max_block_size);

unsigned long int fcomm_get_max_block_size_baselib2(void){
    return fcomm_get_max_block_size(&baselib_heap);
}
EXPORT_SYMBOL(fcomm_get_max_block_size_baselib2);

unsigned long int fcomm_get_max_block_size_fcomm(void){
    return fcomm_get_max_block_size(&rtai_global_heap);
}
EXPORT_SYMBOL(fcomm_get_max_block_size_fcomm);

unsigned long int fcomm_get_max_block_size_large_heap(void){
    return fcomm_get_max_block_size(&large_heap);
}
EXPORT_SYMBOL(fcomm_get_max_block_size_large_heap);

static void task_executor(long data)
{
    long *ptr = (long *)data;
    void (*rt_thread)(void *) = (void (*)(void *))ptr[0];
    void *args = (void *)ptr[1];
    rt_free(ptr);
    rt_thread(args);
}

int fcomm_get_number_of_online_cpus(void)
{
    return num_online_cpus();
}

int fcomm_set_console_performing_char_input(int charInput){
    int ret;
    unsigned long ids[] = {get_parameter(sizeof(int), &charInput)};
    ret = call_remote_function(FUN_ID_CON_PERFORM_CHAR_INPUT, ids, 1);    
    free_parameters(ids, 1);
    return ret;
}

int exec_bash(const char* command)
{
    unsigned long ret;    
    unsigned long ids[] = {get_str_parameter((char *)command)};
    
    ret = call_remote_function(FUN_ID_EXEC_BASH, ids, 1);    
    free_parameters(ids, 1);
    return ret;
}

int fcomm_rt_check_current_stack()
{
    return rt_check_current_stack();
}

long *fcomm_get_stack_pointer(void)
{
    return rt_whoami()->stack;
}

void fcomm_print_sem(SEM *sem)
{
    rt_printk("queue = %p\n", sem->queue);
    rt_printk("magic = %d\n", sem->magic);
    rt_printk("restype = %d\n", sem->restype);
    rt_printk("count = %d\n", sem->count);
    rt_printk("owndby = %p\n", sem->owndby);
    rt_printk("qtype = %d\n", sem->qtype);
    rt_printk("resq = %p\n", sem->resq);
}

int fcomm_rt_task_init(struct rt_task_struct *task, long data, int stack_size, int priority, int uses_fpu, void(*signal)(void))
{
    int ret = rt_task_init(task, task_executor, data, stack_size, priority, uses_fpu, signal);
#ifdef _ENABLE_ROUND_ROBIN_
    if(ret == 0)
        rt_set_sched_policy(task, RT_SCHED_RR, nano2count(RR_QUANTUM_NS));
#endif
    return ret;
}

int fcomm_rt_task_init_cpuid(struct rt_task_struct *task, long data, int stack_size, int priority, int uses_fpu, void(*signal)(void), unsigned int cpuid)
{
    int ret = rt_task_init_cpuid(task, task_executor, data, stack_size, priority, uses_fpu, signal, cpuid);
#ifdef _ENABLE_ROUND_ROBIN_    
    if(ret == 0)
	rt_set_sched_policy(task, RT_SCHED_RR, nano2count(RR_QUANTUM_NS));
#endif
    return ret;    
}

int fcomm_get_errno(void)
{
    return (int)rt_whoami()->msg;    
}

uint16_t fcomm_htons(uint16_t hostlong)
{
    return __constant_htons(hostlong);
}

uint32_t fcomm_htonl(uint32_t hostlong)
{
    return __constant_htonl(hostlong);
}

int fcomm_vprintk(const char *format, va_list ap) {
    int ret = 0;
    char buf[VPRINTK_BUFFER_SIZE];
    ret = vsnprintf(buf, VPRINTK_BUFFER_SIZE, format, ap);
    rt_printk(buf);
    return ret;
}

int fcomm_sscanf(const char *buf, const char *fmt, ...)
{
    va_list args;
    int i;
    va_start(args, fmt);
    i=vsscanf(buf,fmt,args);
    va_end(args);
    return i;
}

int fcomm_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i=vsnprintf(buf,size,fmt,args);
    va_end(args);
    return i;
}
    
int fcomm_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

int fcomm_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i=vsprintf(buf,fmt,args);
    va_end(args);
    return i;
}

int fcomm_strnicmp(const char * s1, const char * s2, size_t len)
{
    return strnicmp (s1, s2, len);
}

void *fcomm_find_function_by_name(char *name, char *moduleName)
{
    unsigned long ret;    
    unsigned long ids[] = {get_str_parameter(name), get_str_parameter(moduleName), get_parameter_no_copy(sizeof(unsigned long))};
    
    call_remote_function(FUN_ID_FIND_FUNC_BY_NAME, ids, 3);    
    ret = *((unsigned long *)rt_named_malloc(ids[2], 0));
    free_parameters(ids, 3);
    return ret == NULL_PARAMETER_ID ? NULL : (void *)ret;
}

int open(const char *pathname, int flags, int mode)
{
    int ret = 0;    
    unsigned long ids[] = {get_str_parameter((char *)pathname), get_parameter(sizeof(int), &flags), get_parameter(sizeof(int), &mode)};
    
    ret = call_remote_function(FUN_ID_OPEN, ids, 3);    
    
    free_parameters(ids, 3);
    return ret;
}

int write(int fd, const void *buf, int count)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd), get_parameter(count, (void *)buf), get_parameter(sizeof(int), &count)};
    
    ret = call_remote_function(FUN_ID_WRITE, ids, 3);    
    free_parameters(ids, 3);
    
    return ret;
}

int close(int fd)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_CLOSE, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

int read(int fd, const void *buf, int count)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd), get_parameter(count, (void *)buf), get_parameter(sizeof(int), &count)};
    
    ret = call_remote_function(FUN_ID_READ, ids, 3);    
    
    memcpy((void *)buf, rt_named_malloc(ids[1], 0), count);
    
    free_parameters(ids, 3);
    
    return ret;
}

int fstat(int filedes, void *buf)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &filedes), get_parameter(STRUCT_STAT_SIZE, (void *)buf)};
    
    ret = call_remote_function(FUN_ID_FSTAT, ids, 2);    
        
    if(ret != -1){
        if(buf == NULL){
            rt_printk("THE BUFFER IS NULL!\n");
        }
        else{            
            memcpy((void *)buf, rt_named_malloc(ids[1], 0), STRUCT_STAT_SIZE);
        }
    }    
    
    free_parameters(ids, 2);
    
    return ret;
}

off_t lseek(int fildes, off_t offset, int whence)
{
    off_t ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &fildes), 
                            get_parameter(sizeof(off_t), &offset),
                            get_parameter(sizeof(int), &whence),
                            get_parameter_no_copy(sizeof(off_t))};
    
    call_remote_function(FUN_ID_LSEEK, ids, 4);
    
    ret = *((off_t *)rt_named_malloc(ids[3], 0));
    
    free_parameters(ids, 4);
    
    return ret;
}

int flock(int fd, int operation)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd), get_parameter(sizeof(int), &operation)};
    
    ret = call_remote_function(FUN_ID_FLOCK, ids, 2);    
        
    free_parameters(ids, 2);
    
    return ret;
}

int remove(const char *pathname)
{
    int ret = 0;    
    unsigned long ids[] = {get_str_parameter((char *)pathname)};
    
    ret = call_remote_function(FUN_ID_REMOVE, ids, 1);    
        
    free_parameters(ids, 1);
    
    return ret;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &nfds),
                            get_parameter(sizeof(fd_set), readfds), 
                            get_parameter(sizeof(fd_set), writefds), 
                            get_parameter(sizeof(fd_set), exceptfds), 
                            get_parameter(sizeof(struct timeval), timeout), 
                            };
                            
    ret = call_remote_function(FUN_ID_SELECT, ids, 5);    
    
    if(readfds != NULL)
        memcpy((void *)readfds, rt_named_malloc(ids[1], 0), sizeof(fd_set));
    if(writefds != NULL)
        memcpy((void *)writefds, rt_named_malloc(ids[2], 0), sizeof(fd_set));
    if(exceptfds != NULL)
        memcpy((void *)exceptfds, rt_named_malloc(ids[3], 0), sizeof(fd_set));
    
    free_parameters(ids, 5);
    
    return ret;
}

int dup(int oldfd)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &oldfd)};
    
    ret = call_remote_function(FUN_ID_DUP, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

int dup2(int oldfd, int newfd)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &oldfd),
                            get_parameter(sizeof(int), &newfd)};
    
    ret = call_remote_function(FUN_ID_DUP_2, ids, 2);
    
    free_parameters(ids, 2);
    
    return ret;
}

int pipe(int filedes[2])
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int) * 2, &filedes)};
    
    ret = call_remote_function(FUN_ID_PIPE, ids, 1);
    
    memcpy((void *)filedes, rt_named_malloc(ids[0], 0), sizeof(int) * 2);
    free_parameters(ids, 1);    
    
    return ret;
}

int socket(int domain, int type, int protocol)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &domain),
                            get_parameter(sizeof(int), &type),
                            get_parameter(sizeof(int), &protocol)};
    
    ret = call_remote_function(FUN_ID_SOCKET, ids, 3);
    
    free_parameters(ids, 3);
    
    return ret;
}

ssize_t send(int s, const void *buf, size_t len, int flags)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(len, (void *)buf),
                            get_parameter(sizeof(int), &len),
                            get_parameter(sizeof(int), &flags)};
    
    ret = call_remote_function(FUN_ID_SEND, ids, 4);
    
    free_parameters(ids, 4);
    
    return ret;
}

ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(len, (void *)buf),
                            get_parameter(sizeof(int), &len),
                            get_parameter(sizeof(int), &flags),
                            get_parameter(tolen, (void *)to),
                            get_parameter(sizeof(socklen_t), &tolen)};
    
    ret = call_remote_function(FUN_ID_SEND_TO, ids, 6);
    
    free_parameters(ids, 6);
    
    return ret;
}

ssize_t recv(int s, const void *buf, size_t len, int flags)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(len, (void *)buf),
                            get_parameter(sizeof(int), &len),
                            get_parameter(sizeof(int), &flags)};
    
    ret = call_remote_function(FUN_ID_RECV, ids, 4);
    if(ret > 0)
        memcpy((void *)buf, rt_named_malloc(ids[1], 0), ret);
    
    free_parameters(ids, 4);
    
    return ret;
}

ssize_t recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(len, (void *)buf),
                            get_parameter(sizeof(int), &len),
                            get_parameter(sizeof(int), &flags),
                            get_parameter(*fromlen, (void *)from),
                            get_parameter(sizeof(socklen_t), fromlen)};
    
    ret = call_remote_function(FUN_ID_RECV_FROM, ids, 6);
    if(ret > 0)
    {
        memcpy((void *)buf, rt_named_malloc(ids[1], 0), ret);
        *fromlen = *((socklen_t *)rt_named_malloc(ids[5], 0));
        if(*fromlen > 0)
            memcpy((void *)from, rt_named_malloc(ids[4], 0), *fromlen);        
    }
    
    free_parameters(ids, 6);
    
    return ret;
}

int fcntl(int fd, int cmd, long arg)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &fd),
                            get_parameter(sizeof(int), &cmd),
                            get_parameter(sizeof(long), &arg)};
    
    ret = call_remote_function(FUN_ID_FCNTL, ids, 3);
    
    free_parameters(ids, 3);
    
    return ret;
}

int setblocking(int sock, int flag)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &sock),
                            get_parameter(sizeof(int), &flag)};
    
    ret = call_remote_function(FUN_ID_SET_SOCKET_BLOCKING, ids, 2);
    free_parameters(ids, 2);
    
    return ret;
}

int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &sockfd),
                            get_parameter(addrlen, (void *)my_addr),
                            get_parameter(sizeof(socklen_t), &addrlen)};
    
    ret = call_remote_function(FUN_ID_BIND, ids, 3);
    if(ret > -1)
        memcpy((void *)my_addr, rt_named_malloc(ids[1], 0), addrlen);
    
    free_parameters(ids, 3);
    
    return ret;
}

int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &sockfd),
                            get_parameter(addrlen, (void *)serv_addr),
                            get_parameter(sizeof(socklen_t), &addrlen)};
    
    ret = call_remote_function(FUN_ID_CONNECT, ids, 3);
    
    free_parameters(ids, 3);
    
    return ret;
}

int listen(int sockfd, int backlog)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &sockfd),
                            get_parameter(sizeof(int), &backlog)};
    
    ret = call_remote_function(FUN_ID_LISTEN, ids, 2);
    
    free_parameters(ids, 2);
    
    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret = 0;
    socklen_t addr_size = (addrlen == NULL ? 0 : *((socklen_t *)addrlen));
    unsigned long ids[] = { get_parameter(sizeof(int), &sockfd),
                            get_parameter(addr_size, (void *)addr),
                            get_parameter(sizeof(socklen_t), (void *)addrlen)};
    
    ret = call_remote_function(FUN_ID_ACCEPT, ids, 3);
    if(ret > -1 && addrlen != NULL)
        memcpy((void *)addr, rt_named_malloc(ids[1], 0), *((socklen_t *)rt_named_malloc(ids[2], 0)));
    
    free_parameters(ids, 3);
    
    return ret;
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
    int ret = 0;
    socklen_t opt_size = (optlen == NULL ? 0 : *((socklen_t *)optlen));
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(sizeof(int), &level),
                            get_parameter(sizeof(int), &optname),
                            get_parameter(opt_size, (void *)optval),
                            get_parameter(sizeof(socklen_t), (void *)optlen)};
    
    ret = call_remote_function(FUN_ID_GET_SOCK_OPT, ids, 5);
    if(ret > -1 && optval != NULL && optlen != NULL)
    {
        memcpy((void *)optval, rt_named_malloc(ids[3], 0), *((socklen_t *)rt_named_malloc(ids[4], 0)));
        memcpy((void *)optlen, rt_named_malloc(ids[4], 0), sizeof(socklen_t));
    }
    
    free_parameters(ids, 5);
    
    return ret;
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(sizeof(int), &level),
                            get_parameter(sizeof(int), &optname),
                            get_parameter(optlen, (void *)optval),
                            get_parameter(sizeof(socklen_t), (void *)&optlen)};
    
    ret = call_remote_function(FUN_ID_SET_SOCK_OPT, ids, 5);
    
    free_parameters(ids, 5);
    
    return ret;
}

int rand(void)
{
    return call_remote_function(FUN_ID_RAND, NULL, 0);
}

int srand(unsigned int seed)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(unsigned int), &seed)};
    
    ret = call_remote_function(FUN_ID_SRAND, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

int mkdir(const char *pathname, mode_t mode)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)pathname),
                            get_parameter(sizeof(mode_t), &mode)};
    
    ret = call_remote_function(FUN_ID_MKDIR, ids, 2);
    
    free_parameters(ids, 2);
    
    return ret;
}

int rmdir(const char *pathname)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)pathname)};
    
    ret = call_remote_function(FUN_ID_RMDIR, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

int unlink(const char *pathname)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)pathname)};
    
    ret = call_remote_function(FUN_ID_UNLINK, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

int gethostname(char *name, size_t len)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(len, (void *)name),
                            get_parameter(sizeof(size_t), &len)};
    
    ret = call_remote_function(FUN_ID_GET_HOSTNAME, ids, 2);
    
    memcpy((void *)name, rt_named_malloc(ids[0], 0), len);
    
    free_parameters(ids, 2);
    
    return ret;
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &s),
                            get_parameter(*namelen, (void *)name),
                            get_parameter(sizeof(socklen_t), namelen)};
    
    ret = call_remote_function(FUN_ID_GET_PEERNAME, ids, 3);
    
    if(ret != -1)
    {
        *namelen = *((socklen_t *)rt_named_malloc(ids[2], 0));
        if(*namelen > 0)
            memcpy((void *)name, rt_named_malloc(ids[1], 0), *namelen);
    }
    
    free_parameters(ids, 3);
    
    return ret;
}

int sethostname(const char *name, size_t len)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)name),
                            get_parameter(sizeof(size_t), &len)};
    
    ret = call_remote_function(FUN_ID_SET_HOSTNAME, ids, 2);
    
    free_parameters(ids, 2);
    
    return ret;
}

int getdomainname(char *name, size_t len)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(len, (void *)name),
                            get_parameter(sizeof(size_t), &len)};
    
    ret = call_remote_function(FUN_ID_GET_DOMAINNAME, ids, 2);
    
    memcpy((void *)name, rt_named_malloc(ids[0], 0), len);
    
    free_parameters(ids, 2);
    
    return ret;
}

int setdomainname(const char *name, size_t len)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)name),
                            get_parameter(sizeof(size_t), &len)};
    
    ret = call_remote_function(FUN_ID_SET_DOMAINNAME, ids, 2);
    
    free_parameters(ids, 2);
    
    return ret;
}

/**
 * This is a quite complex function.
 * To free your memory use something like: 
 * free_parameter_from_ptr(serv->s_name);
 * free_parameter_from_ptr(serv->s_aliases);
 * free_parameter_from_ptr(serv->s_proto);
 * rt_free(serv);
 * where serv is your servent pointer...
 */
int fcomm_getservbyname(const char *name, const char *proto, void* serv)
{
    int ret = 0;
    unsigned long ids[] = { get_str_parameter((char *)name),
                            get_str_parameter((char *)proto),
                            get_parameter(sizeof(struct servent),serv)};

    ret = call_remote_function(FUN_ID_FCOMM_GETSERVBYNAME, ids, 3);
            
    serv = (void*)rt_named_malloc(ids[2], 0);

    free_parameters(ids, 3);
    
    return ret;
}


int fcomm_getservbyport(int port, const char *proto, void* serv)
{
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &port),
                            get_str_parameter((char *)proto),
                            get_parameter(sizeof(struct servent),serv)};
    
    ret = call_remote_function(FUN_ID_FCOMM_GETSERVBYPORT, ids, 3);

    serv = (void*)rt_named_malloc(ids[2], 0);
        
    free_parameters(ids, 3);
    
    return ret;
}

in_addr_t inet_addr(const char *cp)
{
    in_addr_t ret;    
    unsigned long ids[] = {get_str_parameter((char *)cp), get_parameter_no_copy(sizeof(in_addr_t))};
    
    call_remote_function(FUN_ID_INET_ADDR, ids, 2);
    ret = *((in_addr_t *)rt_named_malloc(ids[1], 0));
    
    free_parameters(ids, 2);
    return ret;
}

int inet_aton(const char *cp, struct in_addr *inp)
{
    int ret = 0;
    unsigned long ids[] = {get_str_parameter((char *)cp), get_parameter(sizeof(struct in_addr), inp)};
    
    ret = call_remote_function(FUN_ID_INET_ATON, ids, 2);
    if(ret != 0)
        memcpy(inp, rt_named_malloc(ids[1], 0), sizeof(struct in_addr));
    
    free_parameters(ids, 2);
    return ret;
}

in_addr_t inet_network(const char *cp)
{
    in_addr_t ret;    
    unsigned long ids[] = {get_str_parameter((char *)cp), get_parameter_no_copy(sizeof(in_addr_t))};
    
    call_remote_function(FUN_ID_INET_NETWORK, ids, 2);
    ret = *((in_addr_t *)rt_named_malloc(ids[1], 0));
    
    free_parameters(ids, 2);
    return ret;
}

char *inet_ntoa(char *buffer, int buffer_len, struct in_addr in)
{
    int ret;
    unsigned long ids[] = {get_parameter(buffer_len, (void *)buffer),
    		           get_parameter(sizeof(int), &buffer_len),
                           get_parameter(sizeof(struct in_addr), &in) };
    
    ret = call_remote_function(FUN_ID_INET_NTOA, ids, 3);
    
    if(ret == 0){
        memcpy(buffer, rt_named_malloc(ids[0], 0), buffer_len);
    }
    
    free_parameters(ids, 3);
    
    return (ret == 0) ? buffer : NULL;
}

in_addr_t inet_lnaof(struct in_addr in)
{
    in_addr_t ret;    
    unsigned long ids[] = {get_parameter(sizeof(struct in_addr), &in), get_parameter_no_copy(sizeof(in_addr_t))};
    
    call_remote_function(FUN_ID_INET_LNAOF, ids, 2);
    ret = *((in_addr_t *)rt_named_malloc(ids[1], 0));
    
    free_parameters(ids, 2);
    return ret;
}

in_addr_t inet_netof(struct in_addr in)
{
    in_addr_t ret;    
    unsigned long ids[] = {get_parameter(sizeof(struct in_addr), &in), get_parameter_no_copy(sizeof(in_addr_t))};
    
    call_remote_function(FUN_ID_INET_NETOF, ids, 2);
    ret = *((in_addr_t *)rt_named_malloc(ids[1], 0));
    
    free_parameters(ids, 2);
    return ret;
}

time_t time(time_t *t)
{
    time_t ret = get_fast_time();
    if(t != NULL){
    	memcpy(t, &ret, sizeof(time_t));
    }

    return ret;
}

int getchar(void)
{
    return call_remote_function(FUN_ID_GETCHAR, NULL, 0);
}

int opendir(const char *name)
{
    int ret = 0;    
    unsigned long ids[] = {get_str_parameter((char *)name)};
    
    ret = call_remote_function(FUN_ID_OPENDIR, ids, 1);
    
    free_parameters(ids, 1);
    return ret;
}

int readdir(int fd, struct dirent *d)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd), get_parameter_no_copy(sizeof(struct dirent))};
    
    ret = call_remote_function(FUN_ID_READDIR, ids, 2);
    if(ret != NULL_PARAMETER_ID)
        memcpy(d, rt_named_malloc(ids[1], 0), sizeof(struct dirent));
    
    free_parameters(ids, 2);
    return ret;
}


int closedir(int fd)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_CLOSEDIR, ids, 1);
    
    free_parameters(ids, 1);
    return ret;
}

int stat(const char *path, void *buf)
{
    int ret = 0;    
    unsigned long ids[] = {get_str_parameter((char *)path), get_parameter(STRUCT_STAT_SIZE, (void *)buf)};
    
    ret = call_remote_function(FUN_ID_STAT, ids, 2);    
    
    if(ret != -1)
        memcpy((void *)buf, rt_named_malloc(ids[1], 0), STRUCT_STAT_SIZE);
    
    free_parameters(ids, 2);
    
    return ret;
}

int ftruncate(int fd, off_t length){
    int ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &fd),
                            get_parameter(sizeof(off_t), &length) };

    ret = call_remote_function(FUN_ID_FTRUNCATE, ids, 2);
    free_parameters(ids, 2);
    return ret;
}

int fopen(const char *path, const char *mode)
{
    int ret = 0;    
    unsigned long ids[] = {get_str_parameter((char *)path), get_str_parameter((char *)mode)};
    
    ret = call_remote_function(FUN_ID_FOPEN, ids, 2);    
    
    free_parameters(ids, 2);
    
    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, int fd)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter_no_copy(size * nmemb), 
                            get_parameter(sizeof(size_t), &size),
                            get_parameter(sizeof(size_t), &nmemb), 
                            get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FREAD, ids, 4);    
    
    if(ret > 0)
        memcpy(ptr, rt_named_malloc(ids[0], 0), ret);
    
    free_parameters(ids, 4);
    
    return ret;
}

char *fgets(char *s, int size, int fd)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter_no_copy(size), 
                            get_parameter(sizeof(int), &size),                            
                            get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FGETS, ids, 3);    
    
    if(ret != NULL_PARAMETER_ID)
        memcpy(s, rt_named_malloc(ids[0], 0), size);
        
    free_parameters(ids, 3);
    
    return ret == NULL_PARAMETER_ID ? NULL : s;
}

int fflush(int fd)
{
    int ret = 0;    
    unsigned long ids[] = {get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FFLUSH, ids, 1);
    
    free_parameters(ids, 1);
    
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, int fd)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter(size * nmemb, (void *)ptr), 
                            get_parameter(sizeof(size_t), &size),
                            get_parameter(sizeof(size_t), &nmemb), 
                            get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FWRITE, ids, 4);    
        
    free_parameters(ids, 4);
    
    return ret;
}
    
int fclose(int fd)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FCLOSE, ids, 1);
        
    free_parameters(ids, 1);
    
    return ret;
}

int ferror(int fd)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter(sizeof(int), &fd)};
    
    ret = call_remote_function(FUN_ID_FERROR, ids, 1);
        
    free_parameters(ids, 1);
    
    return ret;
}

int fseek(int fd, long offset, int whence)
{
    int ret = 0;    
    unsigned long ids[] = { get_parameter(sizeof(int), &fd),
                            get_parameter(sizeof(long), &offset),
                            get_parameter(sizeof(int), &whence)};
    
    ret = call_remote_function(FUN_ID_FSEEK, ids, 3);
        
    free_parameters(ids, 3);
    
    return ret;
}

long ftell(int fd)
{
    long ret = 0;
    unsigned long ids[] = { get_parameter(sizeof(int), &fd),
                            get_parameter_no_copy(sizeof(long))};
    
    ret = call_remote_function(FUN_ID_FTELL, ids, 2);
    ret = *((long *)rt_named_malloc(ids[1], 0));
    
    free_parameters(ids, 2);
    
    return ret;
}

char *ctime(char* buffer, int buffer_len, const time_t *timep)
{
    unsigned long ids[] = { get_parameter(buffer_len, (void *)buffer),
    		            get_parameter(sizeof(int), &buffer_len),
                            get_parameter(sizeof(time_t*), (void*)timep)};

    int ret = call_remote_function(FUN_ID_CTIME, ids, 3);

    if(ret == 0){
        memcpy(buffer, rt_named_malloc(ids[0], 0), buffer_len);
    }
    
    free_parameters(ids, 3);
    
    return (ret == 0) ? buffer : NULL;
}

int fcomm_get_internetinfo(char* hostname, char* ipaddr, int size){
    unsigned long ids[] = { get_parameter(size,hostname),
                            get_parameter(size,ipaddr),
                            get_parameter(sizeof(int), &size)};

    int ret = call_remote_function(FUN_ID_GET_INTERNETINFO, ids, 3);

    if (ret == 0) {
        memcpy(hostname, rt_named_malloc(ids[0], 0), size);
        memcpy(ipaddr, rt_named_malloc(ids[1], 0), size);
    } else {
        strcpy(hostname, "unknown");
        strcpy(ipaddr, "0.0.0.0");
    }

    free_parameters(ids,3);
    return 0;

}

int fcomm_gethostbyname(const char* hostname) {
    unsigned long ids[] = { get_str_parameter((char *)hostname) };

    int ret = call_remote_function(FUN_ID_FCOMM_GETHOSTBYNAME, ids, 1);

    free_parameters(ids,1);
    
    return ret;
}

void fcomm_gethostbyaddr(const void* addr, char* buffer, int buffer_len) {
    unsigned long ids[] = { get_parameter(sizeof(void *),(void*)addr),
                            get_parameter(buffer_len,(void*)buffer),
                            get_parameter(sizeof(int),&buffer_len)};

    int ret = call_remote_function(FUN_ID_FCOMM_GETHOSTBYADDR, ids, 3);

    if (ret == 0) {
      memcpy(buffer, rt_named_malloc(ids[1], 0), buffer_len);
    } else {
      strcpy(buffer, "unknown");
    }

    free_parameters(ids,3);
}
