/*
 * socket.c
 * Test Linux ForeHE driver facility 
 * $$
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/atm.h>

typedef struct _RTDNheader{
    unsigned int nSampleNumber; // the sample number since the last t = 0
    unsigned int nSampleTime;   // the time since t = 0, going to PRE as microseconds
} RTDNheader;

#define DEFAULT_VCI 400
#define PACKET	1024
#define MAX_SDU	1024

// probably x86 have an instruction for endianity swap
unsigned int swap32(unsigned int src) {
	unsigned int val;
	((char*)&val)[0] = ((char*)&src)[3];
	((char*)&val)[1] = ((char*)&src)[2];
	((char*)&val)[2] = ((char*)&src)[1];
	((char*)&val)[3] = ((char*)&src)[0]; 
	return  val;
}

// main process 
int main(int argc, char** argv) {
	int _ret;
	int vci;
	int bytesize;
	int keep_run = 1;
	
	// input arguments
	if (argc == 3) {
		sscanf(argv[1], "%d", &vci);
		sscanf(argv[2], "%d", &bytesize);
	}
	else {
		printf("usage: socket [vci bytesize]\n");
		vci = DEFAULT_VCI;
		bytesize = PACKET;
	}
	
	// configuring ATM socket
	printf("listening on vci %d bytesize %d\n", vci, bytesize); 
	int sock = socket(PF_ATMPVC, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("sock");
		return -1;
	}
	struct atm_qos qos;
	memset(&qos, 0, sizeof(qos));
    qos.aal = ATM_AAL5;
	qos.txtp.traffic_class	= ATM_UBR;
	qos.txtp.max_sdu		= MAX_SDU;
	qos.rxtp				= qos.txtp;
	qos.rxtp.spare			= 1;
	_ret = setsockopt(sock, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos));
	if (_ret == -1) {
		perror("setsockopt");
		return -1;
	}
	struct sockaddr_atmpvc addr;
	memset(&addr, 0, sizeof(addr));
	addr.sap_family		= AF_ATMPVC;
	addr.sap_addr.itf	= ATM_ITF_ANY;
	addr.sap_addr.vpi	= 0;
	addr.sap_addr.vci	= vci;
	_ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if (_ret == -1) {
		perror("bind");
		return -1;
	}
	
	//main loop
	int i;
	int last = 0;
	char* data = (char*)malloc(bytesize);
	RTDNheader* header;
	while (keep_run) {
		
		// receiving from socket
		_ret = recv(sock, (void*)data, bytesize, 0);
		if (_ret == -1) {
			perror("recv");
        	break;
		}
		if (_ret != bytesize)
			printf("recv: _ret = %d, bytesize = %d\n", _ret, bytesize);
		
		// checking data
		for ( i=0; i<(_ret/4); i++)
			((int*)data)[i] = swap32(((int*)data)[i]);
		header = (RTDNheader*)data;
		if ((last + 1) != header->nSampleNumber)
			printf("%d %d LOSTS: %d (last: %d now: %d)\n", header->nSampleNumber, header->nSampleTime,
				header->nSampleNumber - (last + 1), last, header->nSampleNumber);
		last = header->nSampleNumber;
		
    	/*printf("%d %d 0x%08x 0x%08x 0x%08x 0x%08x\n", header->nSampleNumber, header->nSampleTime,
    		((int*)data)[0], ((int*)data)[1], ((int*)data)[2], ((int*)data)[3]);
    	*/
    }
	close(sock);	
	return 0;
}

void sender() {
	int _ret;
	int sock;
	_ret = send(sock, 0, 0, 0);
	if (_ret == -1) {
		perror("send");
		return;
	}
}
