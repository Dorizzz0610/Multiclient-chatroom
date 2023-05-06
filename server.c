#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "chatroom.h"
#include <poll.h>


#define MAX 1024 // max buffer size
#define PORT 6789 // server port number
#define MAX_USERS 50 // max number of users
static unsigned int users_count = 0; // number of registered users

static user_info_t *listOfUsers[MAX_USERS] = {0}; // list of users


/* Add user to userList */
void user_add(user_info_t *user);
/* Get user name from userList */
char * get_username(int sockfd);
/* Get user sockfd by name */
int get_sockfd(char *name);

/* Add user to userList */
void user_add(user_info_t *user){
	if(users_count ==  MAX_USERS){
		printf("sorry the system is full, please try again later\n");
		return;
	}
	/***************************/
	/* TODO: add the user to the list */
	/**************************/
	listOfUsers[users_count] = user;
	users_count++;

}

/* Determine whether the user has been registered  */
int isNewUser(char* name) {
	int i;
	int flag = -1;
	/*******************************************/
	/* TODO: Compare the name with existing usernames */
	/*******************************************/
	for(i = 0; i < users_count; i++)
	{
		if(strcmp(listOfUsers[i]->username, name) == 0)
		{
			flag = i;
			break;
		}
	}

	return flag;
}

/* Get user name from userList */
char * get_username(int ss){
	int i;
	static char uname[MAX];
	/*******************************************/
	/* TODO: Get the user name by the user's sock fd */
	/*******************************************/
	for(i = 0; i < users_count; i++)
	{
		if(listOfUsers[i]->sockfd == ss)
		{
			strcpy(uname, listOfUsers[i]->username);
			break;
		}
	}

	return uname;
}

/* Get user sockfd by name */
int get_sockfd(char *name){
	int i;
	int sock;
	/*******************************************/
	/* TODO: Get the user sockfd by the user name */
	/*******************************************/
	for(i = 0; i < users_count; i++)
	{
		if(strcmp(listOfUsers[i]->username, name) == 0)
		{
			sock = listOfUsers[i]->sockfd;
			return sock;
		}
	}


	return -1;
}
// The following two functions are defined for poll()
// Add a new file descriptor to the set
void add_to_pfds(struct pollfd* pfds[], int newfd, int* fd_count, int* fd_size)
{
	// If we don't have room, add more space in the pfds array
	if (*fd_count == *fd_size) {
		*fd_size *= 2; // Double it

		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

	(*fd_count)++;
}
// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int* fd_count)
{
	// Copy the one from the end over this one
	pfds[i] = pfds[*fd_count - 1];

	(*fd_count)--;
}



int main(){
	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	int addr_size;     // length of client addr
	struct sockaddr_in server_addr, client_addr;
	
	char buffer[MAX]; // buffer for client data
	int nbytes;
	int fd_count = 0;
	int fd_size = 5;
	struct pollfd* pfds = malloc(sizeof * pfds * fd_size);
	
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, u, rv;

    
	/**********************************************************/
	/*TODO: create the listener socket and bind it with server_addr*/
	/**********************************************************/
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
	{
		perror("socket");
		exit(1);
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if(bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
	{
		perror("bind");
		exit(1);
	}
	


	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0) {
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening...\n");
	// Add the listener to set
	pfds[0].fd = listener;
	pfds[0].events = POLLIN; // Report ready to read on incoming connection
	fd_count = 1; // For the listener
	// main loop
	for(;;) {
		/***************************************/
		/* TODO: use poll function */
		/**************************************/
		int poll_count = poll(pfds, fd_count, -1);
		if(poll_count == -1)
		{
			perror("poll");
			exit(1);
		}
		else if(poll_count == 0)
		{
			perror("timeout");
			exit(1);
		}

		// run through the existing connections looking for data to read
        	for(i = 0; i < fd_count; i++) {
            	  if (pfds[i].revents & POLLIN) { // we got one!!

                    if (pfds[i].fd == listener) {
                      /**************************/
					  /* TODO: we are the listener and we need to handle new connections from clients */
					  /****************************/
					addr_size = sizeof(client_addr);
					newfd = accept(listener, (struct sockaddr*)&client_addr, &addr_size);
					
					if(newfd < 0)
					{
						perror("accept");
					}
					else //listener gets new connection
					{
						add_to_pfds(&pfds, newfd, &fd_count, &fd_size); 						
						// send welcome message
						bzero(buffer, sizeof(buffer));
						strcpy(buffer, "Welcome to the chat room!\nPlease enter a nickname.\n");
						if (send(newfd, buffer, sizeof(buffer), 0) == -1)
							perror("send");
					}
                      }
                    
					else { // it's the client. handle data from a client
						bzero(buffer, sizeof(buffer));
						// got error or connection closed by client
                        if ((nbytes = recv(pfds[i].fd, buffer, sizeof(buffer), 0)) <= 0) {
                          if (nbytes == 0) {
                            // connection closed
                          } else {
                            perror("recv");
                          }
						  close(pfds[i].fd); // Bye!
						  del_from_pfds(pfds, i, &fd_count);
                        } 
						// we got some data from a client
						else {
							if (strncmp(buffer, "REGISTER", 8)==0){
								printf("Got register/login message\n");
								/********************************/
								/* TODO: Get the user name and add the user to the userlist*/
								/**********************************/
								char name[MAX];
								strcpy(name, buffer+9);

								if (isNewUser(name) == -1) {
									/********************************/
									/* TODO: it is a new user and we need to handle the registration*/
									/**********************************/

									user_info_t *new_user = malloc(sizeof(user_info_t));
									strcpy(new_user->username, name);
									new_user->sockfd = pfds[i].fd;
									new_user->state = 1; // set to online
									user_add(new_user);
									/********************************/
									/* TODO: create message box (e.g., a text file) for the new user */
									/**********************************/
									char filename[MAX];
									strcpy(filename, name);
									strcat(filename, ".txt");
									FILE* message_box = fopen(filename, "w");
									if(message_box == NULL)
									{
										printf("Failed to create message box.");
										exit(1);
									}
									fclose(message_box);

									// broadcast the welcome message (send to everyone except the listener)
									bzero(buffer, sizeof(buffer));
									strcpy(buffer, "Welcome ");
									strcat(buffer, name);
									strcat(buffer, " to join the chat room!\n");
									/*****************************/
									/* TODO: Broadcast the welcome message*/
									/*****************************/
									for(j = 0; j < fd_count; j++) {
										if (pfds[j].fd != listener && pfds[j].fd != pfds[i].fd) {
											if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
												perror("send");
										}
									}


									/*****************************/
									/* TODO: send registration success message to the new user*/
									/*****************************/
									bzero(buffer, sizeof(buffer));
									sprintf(buffer, "Welcome %s to join the chatroom!\n A new account has been created.\n", name);
									if (send(pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
									

								}
								else {
									/********************************/
									/* TODO: it's an existing user and we need to handle the login. Note the state of user,*/
									/**********************************/
									user_info_t* current_user;

									for(int j = 0; j < users_count; j++)
									{

										if(strcmp(listOfUsers[j]->username, name) == 0)
										{
											current_user = listOfUsers[j];
										}
									}
									current_user->state = 1; //online
									current_user->sockfd = pfds[i].fd;//WARNING: update fd
									

									/********************************/
									/* TODO: send the offline messages to the user and empty the message box*/
									/**********************************/
									char login_msg[MAX];
									strcpy(login_msg, "Welcome back! The message box contains:\n");
									send(pfds[i].fd, login_msg, strlen(login_msg), 0);
									
									char filename[MAX];
									strcpy(filename, current_user->username);
									strcat(filename, ".txt");
									FILE* message_box = fopen(filename, "r");
									if(message_box == NULL)
									{
										printf("Failed to load the message box.");
										exit(1);
									}
									char message[MAX];
									while(fgets(message, MAX, message_box) != NULL)
									{
										send(pfds[i].fd, message, strlen(message), 0);
									}
									fclose(message_box);

									//empty the message box
									message_box = fopen(filename, "w");
									if(message_box == NULL)
									{
										printf("Failed to empty the message box.");
										exit(1);
									}

									// broadcast the welcome message (send to everyone except the listener)
									bzero(buffer, sizeof(buffer));
									strcat(buffer, name);
									strcat(buffer, " is online!\n");
									/*****************************/
									/* TODO: Broadcast the welcome message*/
									/*****************************/
									for(int j = 0; j < fd_count; j++)
									{
										if(pfds[j].fd != listener && pfds[i].fd)
										{
											if(send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
											{
												perror("send");
											}
										}
									}

								}
							}
							else if (strncmp(buffer, "EXIT", 4)==0){
								printf("Got exit message. Removing user from system\n");
								// send leave message to the other members
                                bzero(buffer, sizeof(buffer));
								strcpy(buffer, get_username(pfds[i].fd));
								strcat(buffer, " has left the chatroom\n");
								/*********************************/
								/* TODO: Broadcast the leave message to the other users in the group*/
								/**********************************/
								for(int j = 0; j < fd_count; j++)
								{
									if(pfds[j].fd != listener && pfds[j].fd != pfds[i].fd)
									{
										if(send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
										{
											perror("send");
										}
									}
								}

								/*********************************/
								/* TODO: Change the state of this user to offline*/
								/**********************************/
								user_info_t* current_user;

								for(int j = 0; j < users_count; j++)
								{
									if(listOfUsers[j]->sockfd == pfds[i].fd)
									{
										current_user = listOfUsers[j];
									}
								}
								current_user->state = 0; //offline
								current_user->sockfd = 0; //clean the fd
								
								//close the socket and remove the socket from pfds[]
								close(pfds[i].fd);
								del_from_pfds(pfds, i, &fd_count);
							}
							else if (strncmp(buffer, "WHO", 3)==0){
								// concatenate all the user names except the sender into a char array
								printf("Got WHO message from client.\n");
								char ToClient[MAX];
								bzero(ToClient, sizeof(ToClient));
								/***************************************/
								/* TODO: Concatenate all the user names into the tab-separated char ToClient and send it to the requesting client*/
								/* The state of each user (online or offline)should be labelled.*/
								/***************************************/
								int start = 1;

								for(int j = 0; j < users_count; j++)
								{
									if(listOfUsers[j]->sockfd == pfds[i].fd)
									{
										continue;
									}
									
									if(listOfUsers[j]->state == 1)
									{
										if(start == 1)
										{
											start = 0;
											strcat(ToClient, listOfUsers[j]->username);
											strcat(ToClient, "*");
											continue;
										}
										strcat(ToClient, ",\t");
										strcat(ToClient, listOfUsers[j]->username);
										strcat(ToClient, "*");
									}
									else if(listOfUsers[j]->state == 0)
									{
										if(start == 1)
										{
											start = 0;
											strcat(ToClient, listOfUsers[j]->username);
											continue;
										}
										strcat(ToClient, ",\t");
										strcat(ToClient, listOfUsers[j]->username);
									
									}
								}
								strcat(ToClient, "\n* means this user online");
								if(send(pfds[i].fd, ToClient, sizeof(ToClient), 0) == -1)
								{
									perror("send");
								}




							}
							else if (strncmp(buffer, "#", 1)==0){
								// send direct message 
								// get send user name:
								printf("Got direct message.\n");
								// get which client sends the message
								char sendname[MAX];
								// get the destination username
								char destname[MAX];
								// get dest sock
								int destsock;
								// get the message
								char msg[MAX];
								/**************************************/
								/* TODO: Get the source name xx, the target username and its sockfd*/
								/*************************************/
								bzero(sendname, sizeof(sendname));

								printf("Got direct message from client %d.\n", pfds[i].fd);
								strcpy(sendname, get_username(pfds[i].fd));

								char* index1 = strchr(buffer, '#');
								char* index2 = strchr(buffer, ':');
								
								bzero(destname, sizeof(destname));
								strncpy(destname, index1 + 1, index2 - index1 - 1);

								destsock = get_sockfd(destname);
								bzero(msg, sizeof(msg));
								strncpy(msg, index2 + 1, strlen(index2 + 1));

								printf("sendname: %s\n", sendname);
								printf("destname: %s\n", destname);
								printf("destsock: %d\n", destsock);
								printf("msg: %s\n", msg);


								if (destsock == -1) {
									/**************************************/
									/* TODO: The target user is not found. Send "no such user..." messsge back to the source client*/
									/*************************************/
									bzero(msg, sizeof(msg));

									strcpy(msg, "There is no such user. Please check your input format.");

									if(send(pfds[i].fd, msg, sizeof(msg), 0) == -1)
									{
										perror("send");
									}

								}
								else {
									// The target user exists.
									// concatenate the message in the form "xx to you: msg"
									char sendmsg[MAX];
									strcpy(sendmsg, sendname);
									strcat(sendmsg, " to you: ");
									strcat(sendmsg, msg);

									/**************************************/
									/* TODO: According to the state of target user, send the msg to online user or write the msg into offline user's message box*/
									/* For the offline case, send "...Leaving message successfully" message to the source client*/
									/*************************************/
									user_info_t* current_user;
									for(int j = 0; j < users_count; j++)
									{

										if(listOfUsers[j]->sockfd == destsock)
										{
											current_user = listOfUsers[j];
										}
									}
									if(current_user->state == 1)
									{

										if(send(destsock, sendmsg, sizeof(sendmsg), 0) == -1)
										{
											perror("send");
										}
									}
									else
									{
										char filename[MAX];
										strcpy(filename, destname);
										strcat(filename, ".txt");
										FILE* message_box = fopen(filename, "a");
										if(message_box == NULL)
										{
											perror("Failed to load the message box.");
										}
										else
										{
											fprintf(message_box, "%s to you: %s\n", sendname, msg);
											fclose(message_box);
											char success_message[MAX]; 
											strcpy(success_message, destname);
											strcat(success_message, " is offline. Leaving message successfully.");
											if(send(pfds[i].fd, success_message, sizeof(success_message), 0) == -1)
											{
												perror("send");
											}
										}
									}
								
								}
								
								


							}
							else{
								printf("Got broadcast message from user\n");
								/*********************************************/
								/* TODO: Broadcast the message to all users except the one who sent the message*/
								/*********************************************/
								char message[MAX];
								bzero(message, sizeof(message));
								strcpy(message, get_username(pfds[i].fd));
								strcat(message, ": ");
								strcat(message, buffer);
								int send_sock = pfds[i].fd;

								for(int j = 0; j < fd_count; j++)
								{
									if(pfds[j].fd != listener && pfds[j].fd != send_sock)
									{
										if(send(pfds[j].fd, message, sizeof(message), 0) == -1)
										{
											perror("send");
										}
									}
								}
								
							}   

                        }
                    } // end handle data from client
                  } // end got new incoming connection
                } // end looping through file descriptors
        } // end for(;;) 
		

	return 0;
}
