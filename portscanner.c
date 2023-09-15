#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_PORT 65535
#define MIN_PORT 1

/*
 * --------------
 * Usage function
 * --------------
 *
 * Parameter:
 * - @executable: [char *] - executable file
 *
 * Print out the usage of the program
 */
void usage(char *executable)
{
	printf("Usage of port scanner:\n");
	printf("%s <target_ip> <port_range>\n", executable);
	printf("\n\tportscanner [-h help]");
}

/*
 * -------------------
 * Parse_Port function
 * -------------------
 *
 * Parameter:
 * - @port: [const char*] - the port until where we iterate
 *
 * Parse the port range to verify it is between 1 and 65535, and there are
 * no errors in it (alphabetical characters, etc)
 */
unsigned short int parse_port(const char* port)
{
	int port_range;

	if (!port) {
		fprintf(stderr, "Error: no port.");
		exit(EXIT_FAILURE);
	}

	// Checking potential errors
	for (size_t i = 0; i < strlen(port); i++) {
		if (!isdigit(port[i])) {
			fprintf(stderr, "Error: port number goes from 1 to 65535.");
			exit(EXIT_FAILURE);
		}
	}

	// Casting string to integer
	port_range = atoi(port);

	// Checking if it is 1-65535 range
	if (port_range > MAX_PORT || port_range < MIN_PORT) {
		fprintf(stderr, "Error: port number goes from 1 to 65535");
		exit(EXIT_FAILURE);
	}

	return ((unsigned short int)port_range);
}

/*
 * -----------------
 * Parse_IP function
 * -----------------
 *
 * Parameter:
 * - @ip: [char**] - string containing the IP address
 *
 * Parsing the IP address (v4 & v6) and store it in an array of strings
 * One string is the IP, the other one is the version (v4 or v6)
 */
char	**parse_ip(const char *ip)
{
	struct sockaddr_in	sa;
	struct sockaddr_in6	sa6;
	char				**ip_addr;

	if (!(ip_addr = (char**)malloc(sizeof(char*) * 2))) {
		fprintf(stderr, "Error: allocation failed");
		exit(EXIT_FAILURE);
	}

	if (!ip) {
		fprintf(stderr, "Error: no IP address.");
		exit(EXIT_FAILURE);
	}

	// IPv4
	if (inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1) {

		ip_addr[0] = (char*)ip;
		ip_addr[1] = "v4";

		return (ip_addr);
	}
	// IPv6
	else if (inet_pton(AF_INET6, ip, &(sa6.sin6_addr)) == 1)
	{
		ip_addr[0] = (char*)ip;
		ip_addr[1] = "v6";

		return (ip_addr);
	}
	else
	{
		fprintf(stderr, "Error: [argv[1]] - IPv4 or IPv6 needed.\n\
Example: 127.0.0.1 or a1a1:a2a2:a3a3:a4a4:a5a5:a6a6:a7a7:a8a8");
		exit(EXIT_FAILURE);
	}
}

/*
 * ----------------
 * Scanner function
 * ----------------
 *
 * Parameters:
 * - @ip: [char**] - contains ip address and type of address (v4/v6)
 * - @port_range: [unsigned short int] - contains the range until where we scan (1-65535)
 *
 *  Create a socket server
 */
void	scanner(char **ip, unsigned short int port_range)
{
	int 				sockfd = 0;
	struct sockaddr_in	server;

	// Create a socket, depending if IPv4 or IPv6
	if (!strcmp(ip[1], "v4")) {

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Error: socket IPv4");
			exit(EXIT_FAILURE);
		}

		// Set up the server address structure
		server.sin_family = AF_INET;
	}
	else if (!strcmp(ip[1], "v6"))
	{
		if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
			perror("Error: socket IPv6");
			exit(EXIT_FAILURE);
		}

		// Set up the server address structure
		server.sin_family = AF_INET6;
	}

	// Set up the server address structure
	server.sin_addr.s_addr = inet_addr(ip[0]);

	printf("\n-------------");
	printf(" Scanning ports ... ");
	printf("-------------\n");

	// Loop through the specified port range and attempt to connect
	for (unsigned short int port = 1; port <= port_range; port++) {
		server.sin_port = htons(port);

		if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == 0)
			printf("Port %hu is open.\n", port);
		else
			printf("Port %hu is not open.\n", port);
	}

	// Close the socket
	close(sockfd);
}

/*
 * -----------------
 * Starting function
 * -----------------
 *
 * Display the header message at start of program.
 */
void present()
{
	printf("*----------------*\n");
	printf(" Port Scanner\n\n");
	printf(" - Usage: ./executable_name <target-ip> <port-range>\n\n");
	printf(" Starting...\n");
	printf("*----------------*\n\n");
	sleep(1);
}


/*
 * -------------
 * Main function
 * -------------
 *
 * Program that scans ports (1-port range) of an IP address.
 * It prints out the open ports.
 */
int main(int argc, char *argv[])
{
	// Variable that stocks ip address as a string,
	// and the version of IP (v4 or v6)
	char				**ip_addr;

	// The number of port that will be scanned, from 1 to port_range
	unsigned short int	port_range;


	if (!(ip_addr = (char**)malloc(sizeof(char*) * 2))) {
		fprintf(stderr, "Error: allocation failed.");
		exit(EXIT_FAILURE);
	}

	// ./portscanner -h
	if (argc == 2 && !strcmp(argv[1], "-h"))
		usage(argv[0]);
	// scanner part
	else if (argc == 3)
	{
		//Presentation
		present();

		// Parsing IP
		ip_addr = parse_ip(argv[1]);

		// Parsing port range
		port_range = parse_port(argv[2]);

		//----------------
		printf("IP Address: %s\n", ip_addr[0]);
		printf("IP Address type: %s\n", ip_addr[1]);
		printf("Ports: 1-%hu\n", port_range);
		//----------------

		// Scanner
		scanner(ip_addr, port_range);
	}
	else
	{
		fprintf(stderr, "Error: Wrong number of arguments.\nUse -h for more information.\n");
		free(ip_addr);
		exit(EXIT_FAILURE);
	}

	free(ip_addr);

	return (0);
}
