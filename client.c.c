#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SA struct sockaddr
#define SERVER_IP "10.0.2.68" // Server IP address
#define SERVER_PORT 12345
#define TCPPORT 55555
#define MAX_BUFFER_SIZE 61686
#define TIMEOUT_SEC 1 // Timeout in seconds
#define MAX_RETRIES 25600 // Maximum number of retransmission attempts

// Structure to represent a data packet
struct Packet {
    int seq_num;
    char data[MAX_BUFFER_SIZE];
    size_t length;
};

struct sockaddr_in server_addr, servad, cli;
size_t bytes_read;
int seq_num = 0;

void sendpacket(int client_socket, struct Packet send_packet)
{
	send_packet.seq_num = seq_num;
    send_packet.length = bytes_read;
	if (sendto(client_socket, &send_packet, sizeof(struct Packet), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
	{
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int client_socket, sockfd, connfd;
    char buffer[MAX_BUFFER_SIZE];

    // Create UDP socket
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	
	//TCP Socket for ACks
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servad, sizeof(servad)); 

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }
	
	// Assigning IP and Port
	    servad.sin_family = AF_INET;
    	servad.sin_addr.s_addr = inet_addr("10.0.2.68");
    	servad.sin_port = htons(TCPPORT);
	
	if (connect(sockfd, (SA*)&servad, sizeof(servad))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    	}
    else
        printf("connected to the server..\n");

    // Read the file to be sent
    FILE *file = fopen("data.txt", "rb");
    if (file == NULL) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    struct Packet send_packet;
    
	int ack_no,n,test_no;
	// Read data from the file
	bytes_read = fread(send_packet.data, 1, MAX_BUFFER_SIZE, file);
	sendpacket(client_socket, send_packet);
	printf("Sent packet with sequence number %d\n", seq_num);
    while (1) 
	{
		//sendpacket(client_socket, send_packet);
		recv (sockfd, &ack_no, sizeof(ack_no),0);
        if (ack_no == seq_num) 
		{
		// Read data from the file
        printf("Received acknowledgment for packet with sequence number %d\n", ack_no);
		seq_num++; 
		bytes_read = fread(send_packet.data, 1, MAX_BUFFER_SIZE, file);
		if (bytes_read == 0) 
			{
				break; // End of file
			}
		sendpacket(client_socket, send_packet);
		printf("Sent packet with sequence number %d\n", seq_num);
		}
		else
		{
			printf("Resent Packet with ACK : %d\n",ack_no);
			sendpacket(client_socket, send_packet);
		}
    }
    fclose(file);
    close(client_socket);
    close(sockfd);
    return 0;
}