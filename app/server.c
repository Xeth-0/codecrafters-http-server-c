#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

static char *file_directory;

void *handle_connection(void *new_socket_fd);

int main(int argc, char **argv)
{
	if (argc >= 2 && (strcmp(argv[1], "--directory") == 0))
	{
		file_directory = argv[2];
	}
	else
	{
		file_directory = "/tmp";
	}

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
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	// Adding Multi-Threading and an infinite loop to keep the server alive.
	while (1)
	{
		pthread_t new_process;

		new_socket_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (new_socket_fd == -1)
		{
			continue;
		}
		else
		{
			int *p_socket_fd = malloc(sizeof(int));
			if (p_socket_fd == NULL)
			{
				printf("Failed to allocate memory for socket ID\n");
				continue;
			}

			*p_socket_fd = new_socket_fd;
			if (pthread_create(&new_process, NULL, handle_connection, p_socket_fd) != 0)
			{
				printf("Failed to create thread\n");
				free(p_socket_fd);
				continue;
			}
		}

		pthread_detach(new_process);
	}

	close(server_fd);
	return 0;
}

void *handle_connection(void *p_socket_fd)
{
	int new_socket_fd = *((int *)p_socket_fd);

	printf("File Directory: %s\n", file_directory);
	printf("Client connected: Socket - %d\n", new_socket_fd);

	ssize_t val_read;
	char buffer[2048 * 4]; // buffer of size 2048 to store the request.

	// printf("Size of buffer: %lu\n", sizeof(buffer)); // GET requests are limited to 2048 characters.

	// Read the incoming request and store it in the buffer.
	val_read = read(new_socket_fd, buffer, sizeof(buffer) - 1); // -1 on the buffer for the null terminator
	if (val_read == -1 || strlen(buffer) == 0)
	{
		printf("Error reading request into buffer...\n");
		return NULL;
	}
	printf("\nRequest Buffer: \n%s\n\n", buffer);

	// Separate the Request Line, Headers and Request Body
	char *buffer_rest;
	char *request_line = strtok_r(buffer, "\r\n", &buffer_rest);
	char *headers = strtok_r(NULL, "", &buffer_rest); // TODO Currently does not work. headers will also include the body.
	char *request_body = strtok_r(NULL, "\r\n", &buffer_rest);

	// printf("Request Line: %s\nHeaders: %s\nBody: %s\n", request_line, headers, request_body);

	// Parse the request line for the path.
	char *request_line_rest;
	char *req_type = strtok_r(request_line, " ", &request_line_rest);
	char *path = strtok_r(NULL, " ", &request_line_rest); // I have no idea why this works.

	printf("Full Path: %s\n", path);

	char response[1024];

	char *path_tok_rest;
	char *path_tok = strtok_r(path, "/", &path_tok_rest);

	if (strcmp(path, "/") == 0)
	{
		sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
	}
	else if (strcmp(path_tok, "echo") == 0)
	{
		printf("Path found: %s\n", path);

		char *response_body = strtok_r(NULL, "/", &path_tok_rest);

		sprintf(
			response,
			"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s",
			strlen(response_body),
			response_body);

		printf("RESPONSE: %s\n", response);
	}
	else if (strcmp(path_tok, "user-agent") == 0)
	{
		printf("Path found: %s\n", path);
		char *header_rest;

		char *header_line = strtok_r(headers, "\r\n", &header_rest);

		// Iterate over the headers till we find the User-Agent header
		while (header_line != NULL)
		{
			printf("Header Line: %s\n", header_line);

			char *headerline_rest;
			char *header_name = strtok_r(header_line, ": ", &headerline_rest);

			if (strcmp(header_name, "user-agent") == 0 || strcmp(header_name, "User-Agent") == 0)
			{
				// printf("User Agent line: %s\n", header_line);
				char *user_agent = strtok_r(NULL, " ", &headerline_rest);
				printf("User Agent Value: %s\n", user_agent);

				sprintf(
					response,
					"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s",
					strlen(user_agent),
					user_agent);

				break;
			}

			header_line = strtok_r(NULL, "\r\n", &header_rest);
		}
	}
	else if (strcmp(path_tok, "files") == 0)
	{
		char *filename = strtok_r(NULL, "/", &path_tok_rest);
		printf("FileName: %s\n", filename);

		// Construct the file path
		char file_path[strlen(file_directory) + strlen(file_directory) + 20];
		sprintf(
			file_path,
			"%s/%s",
			file_directory,
			filename);
		printf("FILE PATH: %s\n", file_path);

		// Open the file for reading
		FILE *fptr;
		fptr = fopen(file_path, "r");

		if (fptr != NULL)
		{
			char file_contents[500];
			char file_buffer[200];
			while (fgets(file_buffer, sizeof(file_buffer), fptr))
			{
				printf("File Content: %s\n", file_buffer);
				strncat(file_contents, file_buffer, sizeof(file_contents) - strlen(file_contents) - 1);
			}

			sprintf(
				response,
				"HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n%s",
				strlen(file_contents),
				file_contents);

			fclose(fptr);
		}
		else
		{
			printf("File Not found: %s\n", filename);
			sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
		}
	}
	else
	{
		printf("Path Not found: %s\n", strtok(path, "/"));
		sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
	}

	// Send http 200 response
	send(new_socket_fd, response, strlen(response), 0);

	close(new_socket_fd);
}
