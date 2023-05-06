# Multiclient-chatrrom
HKUST 2023Fall COMP4621 (Computer and Communication Networks) Project.
# I. Description of client functionality

## Create client socket and connect to server socket

The client socket is created by

```c
sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

implying the socket is of IPv4 and TCP.

This socket will then be connected to the server socket by firstly setting attributes of `server_addr` and then call `connect()` with this `server_addr`.

## Register/login

I use getchar() to get user input from the terminal. To send a message to inform the server that this message is about the username used for registration/logining, “REGISTER” is concatenated with the username. 

Then `send()` function is called to send this message to the server.

## Receive server message

The function `recv_server_msg_handler` is used to call recv() function to get server message. If the function receives any message from the server, it will print it to the terminal.

To realize the goal of using recv() to receiving server messages and obtaining user’s input in the terminal at the same time, we need to **create two threads**. Since main function is considered as a thread, one more thread is needed in `recv_server_msg_handler`.

1. Before entering the infinite loop in the main function, the thread is declared and initialized like these:

```c
pthread_t recv_server_msg_thread;

	if (pthread_create(&recv_server_msg_thread, NULL, (void *)recv_server_msg_handler, NULL) != 0)
	{
		perror("creating thread");
		exit(1);
	}
```

perror() is called to safely handle with possible error arised in the creating process.

b. After entering the infinite loop, there is a `getchar()` function, continuously waiting to obtain user input in the terminal when the user is using any function of the chatroom. This may cause race condition with `recv_server_msg_handler()` and prevents it from executing the code like printing the message just received from the server.

Therefore, I use mutex to ensure that `recv_server_msg_handler()` will be processed first, so that every time when there is new message from the server, it can be received and printed immediately. The code is like this:

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

for (;;)
	{
		pthread_mutex_lock(&mutex);

		//...recv() and printf()

		pthread_mutex_unlock(&mutex);
	}

	pthread_exit(NULL);
```

## WHO Function

The client code calls `send()` to send the user inputted buffer “WHO” to the server, so that the server will handle this request and send the user list to the client. The client then uses `recv()` function in another thread to get and print the user list.

## Send private message

Similarly, the client code calls `send()` to send the user inputted private message to the server, starting with “#” to tell the server that this is a private message.

## Send broadcast message

If no prefix is in the user input in terminal, then this request is considered as sending broadcast message to the server. Similarly, the client code calls `send()` to send the user inputted broadcast message to the server.

## Exit

The client code calls `send()` to send the user inputed buffer “EXIT” to the server. Then the connection terminates using `close()` and thread terminates using `pthread_exit(NULL)`.

# II. Description of server functionality

## Add user to userlist

There is a maintained array `listOfUsers` whose element is pointer to struct `user_info_t` storing user’s information.

In the function `user_add()`, new user is added to userlist by setting the next element to the last non-null element in the array to the new `user_info_t` and update `users_count` by adding 1.

## Check whether new user

In the function `isNewUser()`, there is a function parameter which is the name of the user we are interested in. So we just iterate all the existing users in the array `listOfUsers` and compare the username of each user (struct `user_info_t` records the username) and the parameter `name`. If we find any match, we return the corresponding index of the user in the userlist, otherwise we return a flag -1 to imply this user is actually a new user.

## Use sockfd to get username

In the function `get_username()`, we iterate through all the users in the userlist to check which user has sockfd equal to the queried sockfd.

## Use username to get sockfd

In the function `get_sockfd()`, we iterate through all the users in the userlist to check which user has username same with the queried username.

## Created listener sokcet

The client socket is created by

```c
sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

implying the socket is of IPv4 and TCP.

This socket will then be connected to the server socket by firstly setting attributes of `server_addr` and then call `bind()` to make the server socket bind with this `server_addr`.

The socket is then set to listen for incoming connections using the function `listen()`.

## Use poll() to handle multiple clients

In order to handle the requests from mutiple clients simultaneously without blocking any one of them, we need to use `poll()` function. We maintain an array `pfds` storing the file descriptors of the listener & clients and their corresponding sockets.

We use an infinite loop to continuously call `poll()` and run through all the file descriptors in `pfds`, checking the revents attribute and whether the flag `POLLIN` is set. If so, there is incoming data now and we need to process it.

If it’s listener now, we call `accept()` to handle new pending connections from the clients.

If it’s the existing client now, we handle the request and data sent from client sockets, as shown in below steps.

## Handle register/login request

We extract the username from the message sent by the user, and calls `isNewUser()` function to check whether it’s a new user.

If it’s a new user, we consider it as a register request, create a `new user_info_t` struct, set the attributes fd and name correspondingly, and set the state online, then store it into the user list by calling `user_add()`. Also, we use C library function `fopen()` and `fclose()` to create a new message box in the format of .txt for this new user. And we broadcast welcome message to every connecting clients by calling `send()`.

If it’s an existing user, we find the user in the userlist and update the file descriptor & user state. Then we use `fopen()` to extract the offline messages in the message box and use `send()` to send the offline messages this user. We then empty the message box by `fopen()` it but writing nothing into it. 

## Handle exit request

We find the user in the userlist and update the file descriptor to a place holder 0 (because its current file descriptor may be reused by another connection) & user state. Then we use `close()` to close the socket connection, and call `del_from_pfds()` to update array `pfds` to no longer record this file descriptor.

## Handle WHO request

`ToClient` is the char array which concatenates all the usernames and will be sent to the requested client.

We iterate user list and check whether the state recorded in the struct is online or offline. If it’s online, we concatenate one more char “*” to imply to the client that this user is online. Like before, `ToClient` is then send() to the requested client.

## Handle private message request

Since the message sent from the client includes the sender’s name, receiver’s name, and the message content, we need to use `strchr()` and `strncpy()` to extract them from different parts of the message. We also call `get_sockfd()` we finished in previous parts to get the destination file descriptor, and this step is used to determine whether the private message sending process is valid. If this function returns that the user isn’t found, we just send error message to the source client. Else, we `send()` the message to the destination client with the message content we just extracted, or `fopen()` the .txt message box and input the message into it.

## Handle broadcast message request

Like the private message case, we just extract the message content from the client’s request and iterate every user in the user list and call `send()` for each one.

# III. Testing

## Compile and run

To compile and run the server & client codes, open the project directory, and run the commands in the Linux terminal:

```bash
make server
./server
```

```bash
make client
./client
```
