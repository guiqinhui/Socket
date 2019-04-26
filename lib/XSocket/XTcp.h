#ifndef _XTCP_H
#define _XTCP_H
#ifdef WIN32

#define SOCKETTCPDLL_API __declspec(dllexport)

#else
#define SOCKETTCPDLL_API 
#endif
#include <string>
class SOCKETTCPDLL_API XTcp
{
public:
	int Sockfd;
	unsigned short SockPort;
	char SockIP[16];

	int Socket_Create(void);
	bool Socket_Connect(const char *pIP,unsigned char port);
	int Socket_Bind(unsigned short SocketPort);
	XTcp Socket_Accetpt(void);
	int Socket_Send(const char *pBuf,int BufLen);
	int Socket_Recv(char *pBuf,int Buflen);
	void Socket_Close(void);
	XTcp();
	virtual ~XTcp();
};
#endif