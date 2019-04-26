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
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#define		closesocket close
#define		strcpy_s	strcpy
#endif
using namespace std;


XTcp::XTcp()
{
#ifdef WIN32
	//1.windows����Ҫ���ض�̬��
	//ָ��winsock�İ汾�ţ����������Ӧ�����ӿ�
	//MAKEWORD(2,2)��ʾ2.2�汾�����ֽ�ָ���ΰ汾�ţ����ֽ�ָ
	WSADATA wsdata;
	int Ret = 0;
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsdata)) != 0)
	{
		//����ʧ��
		printf("WSAStartup failed with error %d\n", Ret);
	}
#endif
	Sockfd = 0;
}
//���������������ģʽ
bool XTcp::Socket_SetBlock(bool IsBlock)
{
	if (Sockfd <= 0)
	{
		printf("Please create socket��");
		return false;
	}

#ifdef WIN32
	unsigned long u1 = 0;
	if (!IsBlock)
	{
		u1 = 1;
	}
	ioctlsocket(Sockfd,FIONBIO,&u1);
#else
	int flags = fcntl(Sockfd, F_GETFL,0);
	if (flags < 0)
	{
		return false;
	}
	if (IsBlock)
	{
		flags &= ~O_NONBLOCK;
	}
	else
	{
		flags |= O_NONBLOCK;
	}
	if (fcntl(Sockfd, F_SETFL, flags) != 0)
	{
		return false;
	}
	return true;
#endif
}
int XTcp::Socket_Create(void)
{
	//2.����socket
	Sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (Sockfd <= 0)
	{
		printf("Create socket failed!");
	}
	printf("Create Socket[%d]\n", Sockfd);
	return Sockfd;
}
bool XTcp::Socket_Connect(const char *pIP, unsigned short port,int timeout)
{
	if (Sockfd <= 0)
	{
		Socket_Create();
	}
	sockaddr_in saddr;
	fd_set fdSet;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(pIP);
	Socket_SetBlock(false);//����Ϊ������ģʽ
	if (connect(Sockfd, (sockaddr*)&saddr, sizeof(saddr)) < 0)
	{
		//ʹ��select���ó�ʱ
		FD_ZERO(&fdSet);//��ʼ����������
		FD_SET(Sockfd, &fdSet);//���ָ������������������
		timeval tm;//���ó�ʱʱ��
		tm.tv_sec = 0;
		tm.tv_usec = timeout * 1000;
		if (select(Sockfd + 1, 0, &fdSet, 0, &tm) < 0)
		{
			printf("connect %s:%d timeout or failed!:%s\n", pIP, port, strerror(errno));
			Socket_SetBlock(true);//�ָ�Ϊ����ģʽ
			return false;
		}
		printf("Select Success\n");
	}
	Socket_SetBlock(true);//�ָ�Ϊ����ģʽ
	printf("Connect %s:%d Success\n", pIP, port);
	return true;
}
int XTcp::Socket_Bind(unsigned short SocketPort)
{
	//�󶨶˿�
	int Ret = 0;
	if (Sockfd <= 0)
	{
		Socket_Create();
	}
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = htonl(0);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(SocketPort);//ת���������ֽ���С��ת��ˣ�
	Ret = ::bind(Sockfd, (sockaddr*)&saddr, sizeof(saddr));
	if (Ret < 0)
	{
		printf("Bind Port %d failed!\n", SocketPort);
		return -1;
	}
	printf("Bind Port %d Success!\n", SocketPort);
	//4.����
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
	//�ͷż��صĿ���Դ
	if (SOCKET_ERROR == WSACleanup())
	{
		printf("WSACleanup failed with error %d\n", WSAGetLastError());
	}
#endif
}
