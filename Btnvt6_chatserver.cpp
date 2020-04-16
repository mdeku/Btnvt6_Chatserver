#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

char* ids[64];
char* id_cp[64];
SOCKET clients[64];
int numClients = 0;

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	fd_set fdread;
	int ret;
	char buf[256];

	while (1)
	{
		printf("Dang cho cac ket noi...\n");
		//SOCKET client = accept(listener, NULL, NULL);
		//printf("Ket noi moi: %d", client);
		char sendBuf[256];
		char cmd[16], id[32], tmp[32];

		const char* errorMsg = "Sai cu phap. Hay nhap lai.\n";
		const char* helloMsg = "Dang nhap theo cu phap \"[client_id:] [your_id]\".\n";

		FD_ZERO(&fdread);
		FD_SET(listener, &fdread);
		for (int i = 0; i < numClients; i++)
			FD_SET(clients[i], &fdread);

		ret = select(0, &fdread, 0, 0, 0);
		if (ret < 0)
			break;

		if (FD_ISSET(listener, &fdread))
		{
			SOCKET client = accept(listener, NULL, NULL);
			send(client, helloMsg, strlen(helloMsg), 0);

			FD_SET(client, &fdread);
			ret = recv(client, buf, sizeof(buf), 0);
			if (ret <= 0)
			{
				// Xoa socket khoi mang clients
				//FD_CLR(client, &fdread);
				//numClients--;
				printf("Client out\n");
				continue;
			}
			buf[ret] = 0;
			printf("Received: %s\n", buf);
			// Kiem tra cu phap
			ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
			if (ret == 2)
			{
				if (strcmp(cmd, "client_id:") == 0)
				{
					const char* okMsg = "Dang nhap thanh cong. Hay nhap thong diep de chuyen tiep.\n";
					send(client, okMsg, strlen(okMsg), 0);


					ids[numClients] = id;
					clients[numClients] = client;
					numClients++;
					continue;
				}
				else {
					send(client, errorMsg, strlen(errorMsg), 0);
					FD_CLR(client, &fdread);
					numClients--;
				}

			}
			else {
				send(client, errorMsg, strlen(errorMsg), 0);
				FD_CLR(client, &fdread);
				numClients--;
			}
		}

		for (int i = 0; i < numClients; i++)
			if (FD_ISSET(clients[i], &fdread))
			{
				ret = recv(clients[i], buf, sizeof(buf), 0);
				SOCKET client = clients[i];
				if (ret <= 0)
				{
					// Xoa socket khoi mang clients
					//printf("Client %s out\n", ids[i]);
					//FD_CLR(client, &fdread);
					/*for(i; i < numClients; i++)
						if (clients[i + 1])
							clients[i] = clients[i + 1];
					numClients--;*/
					continue;
				}
				buf[ret] = 0;
				printf("Received: %s\n", buf);

				// Thuc hien chuyen tiep tin nhan
				client = clients[i];
				ret = sscanf(buf, "%s", cmd);
				if (ret == -1 || cmd[0] != '@')
				{
					send(client, errorMsg, strlen(errorMsg), 0);
				}
				else
				{
					sprintf(sendBuf, "%s: %s", id, buf + strlen(cmd) + 1);

					if (strcmp(cmd, "@all") == 0)
					{
						// Chuyen tiep tin nhan den cac client khac
						for (int i = 0; i < numClients; i++)
							if (clients[i] != client)
								send(clients[i], sendBuf, strlen(sendBuf), 0);
					}
					else
					{
						for (int i = 0; i < numClients; i++)
							if (strcmp(ids[i], cmd + 1) == 0)
								send(clients[i], sendBuf, strlen(sendBuf), 0);
					}
				}
			}

	}
}