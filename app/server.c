#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd, client_addr_len, new_socket_fd;
	struct sockaddr_in client_addr;
	//
	server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET => IPv4 || SOCK_STREAM => TCP || 0 => IP protocol
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,			 // Address family
		.sin_port = htons(4221),		 // Port number
		.sin_addr = {htonl(INADDR_ANY)}, // Internet address(struct)
	}; // No padding provided.

	// (struct sockaddr *)&serv_addr => It's casting the pointer(address)
	// of serv_addr: from type (sockaddr_in *) - to - (sockaddr *)
	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	//
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	new_socket_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (new_socket_fd == -1)
	{
		printf("Accept failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Client connected\n");
	printf("Socket FN %d\n", new_socket_fd);

	ssize_t val_read;
	char buffer[2048 * 4]; // buffer of size 2048 to store the request.
	// GET requests are limited to 2048 characters.

	printf("Size of buffer: %lu\n", sizeof(buffer));
	val_read = read(new_socket_fd, buffer, sizeof(buffer) - 1); // -1 on the buffer for the null terminator
	printf("\nRequest: \n%s\n\n", buffer);

	const char delimiters[2] = " ";

	char *req_type = strtok(buffer, delimiters);
	char *path = strtok(NULL, delimiters); // I have no idea why this works.

	printf("%s\n", path);

	char response[1024];

	if (strcmp(path, "/") == 0)
	{
		sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
	}
	else if (strcmp(strtok(path, "/"), "echo") == 0)
	{
		printf("Path found: %s\n", path);

		char *response_body = strtok(NULL, "/");

		sprintf(
			response, 
			"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
			strlen(response_body), 
			response_body);

		printf("RESPONSE: %s\n", response);
	}
	else
	{
		printf("Path Not found: %s\n", path);
		sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
	}

	// Send http 200 response
	send(new_socket_fd, response, strlen(response), 0);

	close(new_socket_fd);
	close(server_fd);

	return 0;
}
