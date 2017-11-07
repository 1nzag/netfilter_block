#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <errno.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

/* returns packet id */


char host[100];

int is_HTTP(unsigned char* data)
{
	int flag = 0;
	
	if(!strncmp("GET", data, 3))
	{
		flag = 3;
	}
	if(!strncmp("HOST",data,4))
	{
		flag = 4;
	}
	if(!strncmp("HEAD",data,4))
	{
		flag = 4;
	}
	if(!strncmp("PUT",data,4))
	{
		flag = 4;
	}
	if(!strncmp("DELETE",data,4))
	{
		flag = 4;
	}
	if(!strncmp("OPTIONS",data,4))
	{
		flag = 4;
	}
	return flag;
}


int filter(struct nfq_data *tb)
{
	struct ip *ip_header; 
	struct tcphdr *tcp_header;
	unsigned char* data;
	int size;
	int i;
	size = nfq_get_payload(tb, &data);
	ip_header = data;
	//for(i=0 ; i<20; i++)
	//{
	//	printf("%d: %02x   ",i, ((unsigned char*)ip_header)[i]);
	//}
	//printf("\n");
	//printf("%x\n",(unsigned char)ip_header + 9);
	if((unsigned char)(ip_header->ip_p) != 6) //is not tcp
	{
		return 0;
	}
	data  = (unsigned char*)ip_header + ((ip_header -> ip_hl) * 4); // ip_header_size
	tcp_header = data;
	data = (unsigned char*)tcp_header + ((tcp_header -> th_off) * 4); //tcp_header_size
        for(i=0 ; i<20; i++)
	{
		printf("%d: %02x   ",i, ((unsigned char*)data)[i]);
	}
	printf("\n");
	if(!is_HTTP(data))
	{
		return 0;
	}
	if(strstr(data, host))
	{
		printf("%s",data);
		printf("detected\n");
		return 1;
	}
	return 0;
}

static u_int32_t print_pkt (struct nfq_data *tb)
{
	int id = 0;
	struct nfqnl_msg_packet_hdr *ph;
	struct nfqnl_msg_packet_hw *hwph;
	u_int32_t mark,ifi; 
	int ret;
	unsigned char *data;
	
	//printf("aa\n");
	ph = nfq_get_msg_packet_hdr(tb);

	hwph = nfq_get_packet_hw(tb);

	mark = nfq_get_nfmark(tb);

	ifi = nfq_get_indev(tb);
	ifi = nfq_get_outdev(tb);
	ifi = nfq_get_physindev(tb);
	ifi = nfq_get_physoutdev(tb);
	return id;
}
	

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
	      struct nfq_data *nfa, void *data)
{
	u_int32_t id = print_pkt(nfa);
	if(filter(nfa))
	{
		return nfq_set_verdict(qh, id, NF_DROP,0,NULL);
	}
	else
	{	
		//printf("entering callback\n");
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
	}
}

int main(int argc, char **argv)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	struct nfnl_handle *nh;
	int fd;
	int rv;
	char buf[4096] __attribute__ ((aligned));

	strcpy(host, argv[1]);

	printf("opening library handle\n");
	h = nfq_open();
	if (!h) {
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}

	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}

	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
	if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}

	printf("binding this socket to queue '0'\n");
	qh = nfq_create_queue(h,  0, &cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}

	printf("setting copy_packet mode\n");
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}

	fd = nfq_fd(h);

	for (;;) {
		if ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {
			//printf("pkt received\n");
			nfq_handle_packet(h, buf, rv);
			continue;
		}
		/* if your application is too slow to digest the packets that
		 * are sent from kernel-space, the socket buffer that we use
		 * to enqueue packets may fill up returning ENOBUFS. Depending
		 * on your application, this error may be ignored. Please, see
		 * the doxygen documentation of this library on how to improve
		 * this situation.
		 */
		if (rv < 0 && errno == ENOBUFS) {
			printf("losing packets!\n");
			continue;
		}
		perror("recv failed");
		break;
	}

	printf("unbinding from queue 0\n");
	nfq_destroy_queue(qh);

#ifdef INSANE
	/* normally, applications SHOULD NOT issue this command, since
	 * it detaches other programs/sockets from AF_INET, too ! */
	printf("unbinding from AF_INET\n");
	nfq_unbind_pf(h, AF_INET);
#endif

	printf("closing library handle\n");
	nfq_close(h);

	exit(0);
}
