#include <stdio.h>

int is_HTTP(unsigned char* data)
{
	int flag = 0;
	
	if(!strncmp("GET", data, 3))
	{
		flag = 1;
	}
	if(!strncmp("HOST",data,4))
	{
		flag = 1;
	}
	if(!strncmp("HEAD",data,4))
	{
		flag = 1;
	}
	if(!strncmp("PUT",data,4))
	{
		flag = 1;
	}
	if(!strncmp("DELETE",data,4))
	{
		flag = 1;
	}
	if(!strncmp("OPTIONS",data,4))
	{
		flag = 1;
	}
	return flag;
}


int filter(struct nfq_data *tb)
{
	struct ip *ip_header; 
	struct tcphdr *tcp_header;
	unsigned char* data;
	int size;
	size = nfq_get_payload(tb, &data);
	ip_header = data;
	if(ip_header->ip_p != 6) //is not tcp
	{
		return 0;
	}
	tcp_header = ip_header + ((ip_header -> ip_hl) * 4); // ip_header_size
	data = tcp_header + ((tcp_header -> th_off) * 4); //tcp_header_size
	if(!is_HTTP(data))
	{
		return 0;
	}
	return 1;
}

	
	
