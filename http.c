#include <stdio.h>

int is_HTTP(u_char* packet)
{
	int flag = 0;
	
	if(!strncmp("GET", packet, 3))
	{
		flag = 1;
	}
	if(!strncmp("HOST",packet,4))
	{
		flag = 1;
	}
	if(!strncmp("HEAD",packet,4))
	{
		flag = 1;
	}
	if(!strncmp("PUT",packet,4))
	{
		flag = 1;
	}
	if(!strncmp("DELETE",packet,4))
	{
		flag = 1;
	}
	if(!strncmp("OPTIONS",packet,4))
	{
		flag = 1;
	}
	return flag;
}
