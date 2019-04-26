
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "XTcp.h"

using namespace std;
class Tcpthread
{
public:
	XTcp Client;
	void ThreadMain()
	{
		int RecvLen = 0, SendLen = 0;
		char RecvBuf[1024] = { 0 };
		//6.接收客户端数据
		printf("Client[%d]\n", Client.SockPort);
		for (;;)
		{
			RecvLen = Client.Socket_Recv(RecvBuf, sizeof(RecvBuf) - 1);
			if (RecvLen <= 0)
			{
				break;
			}
			RecvBuf[RecvLen] = '\0';
			if (strstr(RecvBuf, "quit") != NULL)
			{
				Client.Socket_Send("quit success!\n", sizeof("quit success!\n"));
				break;
			}
			printf("Recv[%d] %s\n", RecvLen, RecvBuf);
			SendLen = Client.Socket_Send("Recv Ok\n", sizeof("Recv Ok\n"));
		}
		Client.Socket_Close();
		printf("Close Client[%d]!\n",Client.SockPort);
		delete this;
	}

};
int main(int argc, char* argv[])
{
	unsigned short Sock_Port = 8080;
	XTcp ServerTcp;
	XTcp ClientTcp;
	if (argc > 1)
	{
		Sock_Port = atoi(argv[1]);
	}
	ServerTcp.Sockfd = ServerTcp.Socket_Create();
	ServerTcp.Socket_Bind(Sock_Port);
	
	for (;;)
	{
		ClientTcp = ServerTcp.Socket_Accetpt();
		Tcpthread *pthread = new Tcpthread();
		pthread->Client = ClientTcp;
		//创建线程
		thread sth(&Tcpthread::ThreadMain,pthread);
		sth.detach();//释放父线程拥有的子线程资源，子线程脱离主线程，不受父线程控制
	}
	ServerTcp.Socket_Close();
	printf("Close Socket!\n");
	getchar();
}
