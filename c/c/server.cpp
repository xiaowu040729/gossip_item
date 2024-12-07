#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<map>
#define MaxEvents 10
#define maxi_connect 1024
#define Epoll_Size 100
#include<iostream>
using namespace std;
/*
������־��ʵ��һ�������ҹ��� ���ڶ��û����� 
ʵ�ֵļ�����ͨ��epoll ʵ�ֶ�����������

*/


struct Client {
	int fd;
	string name;
};
std::map<int, Client> Clients;


int main()
{
	//����һ��epollʵ��
	int epoll_size = Epoll_Size;			 
	int efd = epoll_create(epoll_size);
	if (efd == -1)
	{
		perror("efd error");
		return -1;
	}



	//�����׽���
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		perror("socket");
		return -1;
	}

	//��
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = INADDR_ANY;
	socklen_t len = sizeof(addr);
	int ret = bind(fd, (struct sockaddr*)&addr, len);
	if (fd < 0)
	{
		perror("bind");
		return -1;
	}



	//�������Կͻ��˵�����

	ret = listen(fd, maxi_connect);
	if (ret < 0)
	{
		perror("listen");
		return -1;
	}


	//��������socket����epoll�������
	struct epoll_event ev;
	epoll_event events[MaxEvents];
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		perror("epoll_ctl");
	}


	//ѭ������Ƿ����¼�����
	int timeout = 100;
	while (1)
	{
		int nfd = epoll_wait(efd, events, MaxEvents, timeout); //������¼��Ķ������¼�����
		if (nfd == -1)
		{
			perror("epoll_wait");
		}
		else if (nfd == 0)
		{
			continue;		//���û��ʱ�䷢���Ͱ�������ѭ��������һ�μ��
		}
		else {
			//�ж����¼����� ���������������� ������Ϣ		
			for (int i = 0; i < nfd; i++)
			{
				int fdt = events[i].data.fd;
				//����Ǽ�����socket�յ���Ϣ
				if (fdt == fd)
				{
					struct sockaddr_in saddr;
					socklen_t len = sizeof(saddr);
					int client_fd =accept(fd, (struct sockaddr*)&saddr, &len);
					if (client_fd < 0)
					{
						perror("accept");
						continue;
					}
					

					//�ٰѿͻ��˵�socket����epoll
					struct epoll_event ev_client;
					ev_client.events = EPOLLIN;
					ev_client.data.fd = client_fd;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &ev_client) == -1)
					{
						perror("epoll_ctl_client");
						close(client_fd);
						continue;
					}

					//����ͻ�����Ϣ
					Client client;
					client.fd= client_fd;
					client.name = "";
					Clients[client_fd] = client;
				}
				else {		// �ͻ��˷���Ϣ����������
					char buff[1024];
					int n =read(fdt, buff, sizeof(buff));	//����Ϣ������
					if (n < 0)		//����
					{
						break;
					}
					else if (n == 0)
						{
						//�ͻ��˶Ͽ�����
						close(fdt);
						epoll_ctl(efd, EPOLL_CTL_DEL, fdt,0);	//������epoll����ɾ��
						Clients.erase(fdt);		//�����ӿͻ�����ɾ
						}
					else {
						string msg(buff, n);	//����Ϣת����string

						//�������Ϊ����˵�������û���
						if (Clients[fdt].name == "")
						{
							Clients[fdt].name = msg;

						}
						else {
							string name = Clients[fdt].name;
							//����Ϣ�����ͻ���
							for (auto& c : Clients)
							{
								write(c.first, ('[' + name + ']' + ": " + msg).c_str(), msg.size() + name.size()+ 4);
							}
						}

					}
					
				}


			}
		}
	}


	//�ر�epoll���׽���
	close(fd);
	close(efd);



	return 0;
}