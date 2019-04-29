
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <sys/epoll.h>
#include "XTcp.h"
#define	EPOLL_CTL	1
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
#ifdef EPOLL_CTL
	int epollfd = epoll_create(256);//创建epoll
	int Event_Count = 0;
	char RecvBuf[1024] = { 0 };
	char SendBuf[] = { "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nX" };
	epoll_event epollev;
	epoll_event events[20];
	epollev.data.fd = ServerTcp.Sockfd;
	epollev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD, ServerTcp.Sockfd,&epollev);
	ServerTcp.Socket_SetBlock(false);
#endif
	for (;;)
	{
#ifdef EPOLL_CTL
		Event_Count = epoll_wait(epollfd,events,20,200);
		for (int i = 0; i < Event_Count; i++)
		{
			if ((events[i].data.fd == ServerTcp.Sockfd)
				&&(ServerTcp.Sockfd!=0))
			{
				while (1)
				{
					ClientTcp = ServerTcp.Socket_Accetpt();
					if (ClientTcp.Sockfd <= 0)break;
					epollev.data.fd = ClientTcp.Sockfd;
					epollev.events = EPOLLIN | EPOLLET;
					epoll_ctl(epollfd, EPOLL_CTL_ADD, ClientTcp.Sockfd, &epollev);
				}
			}
			else
			{
				XTcp Client;
				Client.Sockfd = events[i].data.fd;
				Client.Socket_Recv(RecvBuf,sizeof(RecvBuf));
				printf("Recv Clietn[%d]:%s\n", Client.Sockfd,RecvBuf);
				Client.Socket_Send(SendBuf, sizeof(SendBuf));
				//Client.Socket_Close();
			}
		}
#else
		ClientTcp = ServerTcp.Socket_Accetpt();
		Tcpthread *pthread = new Tcpthread();
		pthread->Client = ClientTcp;
		//创建线程
		thread sth(&Tcpthread::ThreadMain,pthread);
		sth.detach();//释放父线程拥有的子线程资源，子线程脱离主线程，不受父线程控制
#endif
	}
	ServerTcp.Socket_SetBlock(true);
	ServerTcp.Socket_Close();
	printf("Close Socket!\n");
	getchar();
}
