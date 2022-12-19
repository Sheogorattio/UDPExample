#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include<deque>
#include<fstream>
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

SOCKET server_socket;

vector<string> history;

struct Client {
	SOCKET socket;
	string nickname;
	string colorMode = "\033[";
};

deque<Client> ClientQ;

int main() {
	history.push_back("Chat was started");

	system("title Server");

	puts("Start server... DONE.");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	listen(server_socket, MAX_CLIENTS);

	puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

	fd_set readfds;

	while (true) {
		FD_ZERO(&readfds);

		FD_SET(server_socket, &readfds);

		for (int i = 0; i < ClientQ.size(); i++)
		{
			SOCKET s = ClientQ[i].socket;
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		Client obj; // new client socket
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((obj.socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}

			send(obj.socket, "Enter your nickname", 20, 0);
			char client_message[DEFAULT_BUFLEN];

			int client_message_length = recv(obj.socket, client_message, DEFAULT_BUFLEN, 0);
			client_message[client_message_length] = '\0';

			obj.nickname = client_message;


			send(obj.socket, "Enter your color code:\nBlack\t30\nRed\t31\nGreen\t32\nYellow\t33\nBlue\t34\nMagenta\t35\nCyan\t36\nWhite\t37", 20, 0);

			client_message_length = recv(obj.socket, client_message, DEFAULT_BUFLEN, 0);
			client_message[client_message_length] = '\0';

			obj.colorMode.append(client_message);
			obj.colorMode.append("m");


			for (int i = 0; i < ClientQ.size(); i++)//sending color mode
			{
				if (ClientQ[i].socket != 0)
				{
					send(ClientQ[i].socket, obj.colorMode.c_str(), obj.colorMode.size(), NULL);
				}
			}

			string NewClientMessage;//sending message
			for (int i = 0; i < obj.nickname.size(); i++) {
				NewClientMessage.push_back(obj.nickname[i]);
			}
			NewClientMessage.push_back(' ');
			char join[] = "joined";
			for (int i = 0; i < strlen(join); i++) {
				NewClientMessage.push_back(join[i]);
			}
			NewClientMessage.push_back('\n');
			
			for (int i = 0; i < ClientQ.size(); i++) 
			{
				if (ClientQ[i].socket != 0) 
				{
					send(ClientQ[i].socket, NewClientMessage.c_str(), NewClientMessage.size(), NULL);
				}
			}
			

			for (int i = 0; i < history.size(); i++)
			{
				cout << history[i] << "\n";
				send(obj.socket, history[i].c_str(), history[i].size(), 0);
			}
			history.push_back(NewClientMessage);
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", obj.socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			ClientQ.push_back(obj);
		}


		for (int i = 0; i < ClientQ.size(); i++)
		{
			SOCKET s = ClientQ[i].socket;
			if (FD_ISSET(s, &readfds))
			{
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);


				char client_message[DEFAULT_BUFLEN];

				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';

				string check_exit = client_message;
				if (check_exit == "off")
				{
					cout << "Client #" << i << " is off\n";
					deque<Client>::iterator iter = ClientQ.begin();
					for ( int j = 0; j < i; j++) iter++;
					ClientQ.erase(iter);
				}
				string temp = ClientQ[i].colorMode;

				for (int i = 0; i < ClientQ.size(); i++)//sending color mode
				{
					if (ClientQ[i].socket != 0)
					{
						send(ClientQ[i].socket, obj.colorMode.c_str(), obj.colorMode.size(), NULL);
					}
				}

				temp= ClientQ[i].nickname;
				temp.push_back(':');
				temp.push_back(' ');
				for (int i = 0; i < strlen(client_message); i++)
				{
					temp.push_back(client_message[i]);
				}
				
				history.push_back(temp);

				for (int i = 0; i < ClientQ.size(); i++) {
					if (ClientQ[i].socket != 0) {
						send(ClientQ[i].socket, history.back().c_str(), history.back().size(), 0);
					}
				}

			}
		}
	}

	WSACleanup();
}