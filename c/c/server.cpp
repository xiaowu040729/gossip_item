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
开发日志：实现一个聊天室功能 用于多用户聊天 
实现的技术：通过epoll 实现多人在线聊天

*/


struct Client {
	int fd;
	string name;
};
std::map<int, Client> Clients;


int main()
{
	//创建一个epoll实例
	int epoll_size = Epoll_Size;			 
	int efd = epoll_create(epoll_size);
	if (efd == -1)
	{
		perror("efd error");
		return -1;
	}



	//创建套接字
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		perror("socket");
		return -1;
	}

	//绑定
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



	//监听来自客户端的请求

	ret = listen(fd, maxi_connect);
	if (ret < 0)
	{
		perror("listen");
		return -1;
	}


	//将监听的socket放入epoll检测树上
	struct epoll_event ev;
	epoll_event events[MaxEvents];
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		perror("epoll_ctl");
	}


	//循环检测是否有事件发生
	int timeout = 100;
	while (1)
	{
		int nfd = epoll_wait(efd, events, MaxEvents, timeout); //检测有事件的东西和事件数量
		if (nfd == -1)
		{
			perror("epoll_wait");
		}
		else if (nfd == 0)
		{
			continue;		//如果没有时间发生就把它跳过循环进入下一次检测
		}
		else {
			//有多少事件发生 看它是连接请求还是 发送消息		
			for (int i = 0; i < nfd; i++)
			{
				int fdt = events[i].data.fd;
				//如果是监听的socket收到消息
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
					

					//再把客户端的socket加入epoll
					struct epoll_event ev_client;
					ev_client.events = EPOLLIN;
					ev_client.data.fd = client_fd;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &ev_client) == -1)
					{
						perror("epoll_ctl_client");
						close(client_fd);
						continue;
					}

					//储存客户端信息
					Client client;
					client.fd= client_fd;
					client.name = "";
					Clients[client_fd] = client;
				}
				else {		// 客户端发信息而不是连接
					char buff[1024];
					int n =read(fdt, buff, sizeof(buff));	//把信息读出来
					if (n < 0)		//出错
					{
						break;
					}
					else if (n == 0)
						{
						//客户端断开连接
						close(fdt);
						epoll_ctl(efd, EPOLL_CTL_DEL, fdt,0);	//把它从epoll树上删除
						Clients.erase(fdt);		//把它从客户端里删
						}
					else {
						string msg(buff, n);	//把信息转化成string

						//如果名字为空则说明这是用户名
						if (Clients[fdt].name == "")
						{
							Clients[fdt].name = msg;

						}
						else {
							string name = Clients[fdt].name;
							//把信息发给客户端
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


	//关闭epoll和套接字
	close(fd);
	close(efd);



	return 0;
}