#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#include<stdio.h>
#define Buffsize  1024


char szMsg[Buffsize];       //���͵���Ϣ
unsigned SendMsg(void* arg)
{
    SOCKET sock = *(SOCKET*)arg;
    while (1)       //ѭ������Ϣ          
    {
        scanf("%s", szMsg);
        if (!strcmp(szMsg, "QUIT\N") || !strcmp(szMsg, "quit\n"))
        {
            closesocket(sock);
            exit(0);
        }
        send(sock, szMsg, strlen(szMsg), 0);
    }
    return 0;
}

unsigned ReceiveMsg(void* arg)
{
    SOCKET sock = *(SOCKET*)arg;
    char buf[Buffsize];
    while (1)       //ѭ������Ϣ       '   
    {
        int len  =recv(sock, buf, sizeof(buf) - 1, 0);
        if (len == -1)
        {
            printf("�������Ͽ�����\n");
            return  -1;
        }
        buf[len] = '\0';
        printf("%s\n", buf);
    }


}

int main()
{
    //��ʼ�����绷��
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {                          
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return -1;
    }

    //�����׽���
    SOCKET hsocket;
    hsocket = socket(AF_INET, SOCK_STREAM, 0);

    //��
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    inet_pton(AF_INET, "159.75.188.178", &addr.sin_addr);
    //���ӷ�����
    int ret = connect(hsocket, (SOCKADDR*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        perror("connect");
    }
    else {
        printf("��ӭ����������������ң��������û�����\n");
    }
   
    //���������߳� һ������Ϣ һ��������Ϣ
    HANDLE  hSendhand = CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)SendMsg, (void*)&hsocket, 0 ,NULL);
    HANDLE  hReceivehand = CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)ReceiveMsg, (void*)&hsocket, 0,NULL);
    

    //�ȴ������߳̽����ٽ���
    WaitForSingleObject(hSendhand, INFINITE);
    WaitForSingleObject(hReceivehand, INFINITE);

    closesocket(hsocket);
    WSACleanup();
	return 0;
}