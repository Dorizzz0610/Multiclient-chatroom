#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chatroom.h"

#define MAX 1024  // max buffer size
#define PORT 6789 // port number

static int sockfd;

void generate_menu()
{
	printf("Hello dear user pls select one of the following options:\n");
	printf("EXIT\t-\t Send exit message to server - unregister ourselves from server\n");
	printf("WHO\t-\t Send WHO message to the server - get the list of current users except ourselves\n");
	printf("#<user>: <msg>\t-\t Send <MSG>> message to the server for <user>\n");
	printf("Or input messages sending to everyone in the chatroom.\n");
}

void recv_server_msg_handler()
{
	/********************************/
	/* TODO: receive message from the server and desplay on the screen*/
	/**********************************/
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	char buffer[MAX];

	for (;;)
	{
		pthread_mutex_lock(&mutex);

		bzero(buffer, sizeof(buffer));
		int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
		if (nbytes == -1)
		{
			perror("Failed to handle message from server\n");	

			pthread_mutex_unlock(&mutex);

			exit(1);
					
		}
		else if (nbytes == 0)
		{
			perror("Server has closed the connection");

			pthread_mutex_unlock(&mutex);

			exit(1);
		}
		else
		{
			printf("%s\n", buffer);
		}

		pthread_mutex_unlock(&mutex);
	}
	
	pthread_mutex_destroy(&mutex);

	pthread_exit(NULL);
}

int main()
{
	int n;
	int nbytes;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];

	/******************************************************/
	/* TODO: create the client socket and connect to the server */
	/******************************************************/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(1);
	}
	else
	{
		printf("Client socket created successfully!\n");
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	// inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("Failed to connect with the server...");
		exit(1);
	}
	else
	{
		printf("Successfully connected to the server!");
	}

	generate_menu();
	// receive welcome message to enter the nickname
	bzero(buffer, sizeof(buffer));
	if (nbytes = recv(sockfd, buffer, sizeof(buffer), 0) == -1)
	{
		perror("recv");
		exit(1);
	}
	printf("%s\n", buffer);
	/*************************************/
	/* TODO: Input the nickname and send a message to the server */
	/* Note that we concatenate "REGISTER" before the name to notify the server it is the register/login message*/
	/*******************************************/
	printf("Enter your nickname: ");

	bzero(buffer, sizeof(buffer));
	n = 0;
	while ((buffer[n++] = getchar()) != '\n')
		;
	buffer[n - 1] = '\0';
	char result[MAX];
	strcpy(result, "REGISTER ");
	strcat(result, buffer);
	strcpy(buffer, result);

	if (send(sockfd, buffer, strlen(buffer), 0) == -1)
	{
		perror("send");
		exit(1);
	}

	// receive welcome message "welcome xx to joint the chatroom. A new account has been created." (registration case) or "welcome back! The message box contains:..." (login case)
	bzero(buffer, sizeof(buffer));
	if (recv(sockfd, buffer, sizeof(buffer), 0) == -1)
	{
		perror("recv");
	}
	printf("%s\n", buffer);

	/*****************************************************/
	/* TODO: Create a thread to receive message from the server*/
	/* pthread_t recv_server_msg_thread;*/
	/*****************************************************/
	pthread_t recv_server_msg_thread;

	if (pthread_create(&recv_server_msg_thread, NULL, (void *)recv_server_msg_handler, NULL) != 0)
	{
		perror("creating thread");
		exit(1);
	}



	// chat with the server
	for (;;)
	{
		bzero(buffer, sizeof(buffer));
		n = 0;

		while ((buffer[n++] = getchar()) != '\n')
		;

		buffer[n - 1] = '\0';			

		if ((strncmp(buffer, "EXIT", 4)) == 0)
		{
			printf("Client Exit...\n");
			/********************************************/
			/* TODO: Send exit message to the server and exit */
			/* Remember to terminate the thread and close the socket */
			/********************************************/
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("send");
				exit(1);
			}
			close(sockfd);
			pthread_exit(NULL);
			break;
		}
		else if (strncmp(buffer, "WHO", 3) == 0)
		{
			printf("Getting user list, pls hold on...\n");
			// pthread_mutex_lock(&mutex);
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("send");
				exit(1);
			}
			// pthread_mutex_unlock(&mutex);
			printf("If you want to send a message to one of the users, pls send with the format: '#username:message'\n");
		}
		else if (strncmp(buffer, "#", 1) == 0)
		{
			// If the user want to send a direct message to another user, e.g., aa wants to send direct message "Hello" to bb, aa needs to input "#bb:Hello"
			
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("send");
				exit(1);
			}
			
		}
		else
		{
			/*************************************/
			/* TODO: Sending broadcast message. The send message should be of the format "username: message"*/
			/**************************************/
			
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("send");
				exit(1);
			}
			// pthread_mutex_unlock(&mutex);
		}
	}
	return 0;
}
