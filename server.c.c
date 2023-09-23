#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SA struct sockaddr
#define PORTUDP 12345
#define PORTTCP 55555
#define MAX_BUFFER_SIZE 61686
#include <time.h>

// Structure to represent a data packet
struct Packet {
    int seq_num;
    char data[MAX_BUFFER_SIZE];
    size_t length;
};

int main() {
	double time_spent = 0;

    int server_socket,sockfd,connfd,len;
    struct sockaddr_in server_addr, client_addr, servad , cli;;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];
	int ack_no = 0;

    // Create UDP socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	
	//Creating TCP Socket for ACKs
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servad, sizeof(servad));
	
	// Assigning IP and Port
	servad.sin_family = AF_INET;
	servad.sin_addr.s_addr = INADDR_ANY;
	servad.sin_port = htons(PORTTCP);

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORTUDP);

	// Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servad, sizeof(servad))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
	
	// Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accepted the client...\n");

    // Bind socket to address and port
    if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
	else
	{
		printf("UDP Bind Success!\n");
	}
	
    int expected_seq_num = 0,n;
    //struct Packet ack_packet;
    socklen_t client_addr_len = sizeof(client_addr);
	
	FILE *file = fopen("received_file.txt", "ab");
    if (file == NULL) 
	{
        perror("File open failed");
        exit(EXIT_FAILURE);
    }
	struct timeval timeout;
	
    while (1) 
	{
		clock_t begin = clock();
        // Receive data from the client
        struct Packet received_packet;
        timeout.tv_sec = 3; // 3 seconds timeout
        timeout.tv_usec = 0;
        setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		
        ssize_t bytes_received = recvfrom(server_socket, &received_packet, sizeof(struct Packet), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received < 0) {
            perror("Receive failed");
			n = send (connfd, &ack_no, sizeof(ack_no), 0);
            printf("Timeout!! Resent ACK : %d!!\n",ack_no);
        }
        // Check if the received packet has the expected sequence number
        if (received_packet.seq_num == expected_seq_num) 
		{
            fwrite(received_packet.data, 1, received_packet.length, file);
			//ACK Send TCP
			ack_no = expected_seq_num;
			n = send (connfd, &ack_no, sizeof(ack_no), 0);
            printf("Received packet with sequence number %d\n", expected_seq_num);
            expected_seq_num++;
        } 
		else 
		{
			printf("Resent ACK : %d\n",ack_no);
			n = send (connfd, &ack_no, sizeof(ack_no), 0);
        }
		clock_t end = clock();
		time_spent = time_spent+((double)(end - begin) / CLOCKS_PER_SEC);
		printf("Time Taken: %ld\n",time_spent);
    }
	close(sockfd);
	fclose(file);
    return 0;
}