#include "XTcp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#define socklen_t int
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define  closesocket close
#endif
using namespace std;


XTcp::XTcp()
{
#ifdef WIN32
	//1.windows下需要加载动态库
	//指定winsock的版本号，并载入其对应的链接库
	//MAKEWORD(2,2)表示2.2版本，高字节指定次版本号，低字节指
	WSADATA wsdata;
	int Ret = 0;
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsdata)) != 0)
	{
		//载入失败
		printf("WSAStartup failed with error %d\n", Ret);
	}
#endif
}

int XTcp::Socket_Create(void)
{
	//2.创建socket
	Sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (Sockfd < 0)
	{
		printf("Create socket failed!");
	}
	printf("Socket[%d]\n", Sockfd);
	return Sockfd;
}
bool XTcp::Socket_Connect(const char *pIP, unsigned char port)
{
	if (Sockfd <= 0)
	{
		Socket_Create();
	}
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(pIP);
	if (connect(Sockfd, (sockaddr*)&saddr, sizeof(saddr)) !=0)
	{
		printf("Connect %s:%d failed!Err\n",pIP,port);
		return false;
	}
	return true;
}
int XTcp::Socket_Bind(unsigned short SocketPort)
{
	//绑定端口
	int Ret = 0;
	if (Sockfd <= 0)
	{
		Socket_Create();
	}
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = htonl(0);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(SocketPort);//转换成网络字节序（小端转大端）
	Ret = ::bind(Sockfd, (sockaddr*)&saddr, sizeof(saddr));
	if (Ret < 0)
	{
		printf("Bind Port %d failed!\n", SocketPort);
		return -1;
	}
	printf("Bind Port %d Success!\n", SocketPort);
	//4.监听
	Ret = listen(Sockfd, 10);
	if (Ret < 0)
	{
		printf("Listen failed!");
		return -2;
	}
	printf("Listening Port %d...\n", SocketPort);
	return 0;
}
XTcp XTcp::Socket_Accetpt(void)
{
	XTcp Tcp;
	int client;
	sockaddr_in caddr;
	socklen_t len = sizeof(caddr);
	if (Sockfd <= 0) return Tcp;
	client = accept(Sockfd, (sockaddr*)&caddr, &len);
	char *IP = inet_ntoa(caddr.sin_addr);
	strcpy_s(Tcp.SockIP,IP);
	Tcp.SockPort = ntohs(caddr.sin_port);
	Tcp.Sockfd = client;
	printf("Accept clent %d(IP:%s,Port:%d)\n", client, Tcp.SockIP, Tcp.SockPort);
	return Tcp;
}

int XTcp::Socket_Send(const char *pBuf, int BufLen)
{
	int SendLen = 0;
	int SendLenPrePacket = 0;
	if (Sockfd <= 0) return -1;
	while (SendLen < BufLen)
	{
		SendLenPrePacket = send(Sockfd, pBuf+ SendLen, BufLen-SendLen, 0);
		if (SendLenPrePacket < 0)
		{
			break;
		}
		SendLen += SendLenPrePacket;
	}
	return SendLen;
	
}
int XTcp::Socket_Recv(char *pBuf, int Buflen)
{
	if (Sockfd <= 0) return -1;
	return recv(Sockfd, pBuf, Buflen, 0);
}
void XTcp::Socket_Close(void)
{
	if (Sockfd <= 0) return;
	closesocket(Sockfd);
}
XTcp::~XTcp()
{
#ifdef WIN32	
	//释放加载的库资源
	if (SOCKET_ERROR == WSACleanup())
	{
		printf("WSACleanup failed with error %d\n", WSAGetLastError());
	}
#endif
}
