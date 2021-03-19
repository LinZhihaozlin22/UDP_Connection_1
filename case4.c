/*
 Student ID: 1607869
 Name: Zhihao Lin

 This the source code of client that emulating case-4 error (duplicated packet) of Assignment 1
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <string.h>
 #include <unistd.h>
 #include <netdb.h>
 #include <poll.h>

 //---- primitives ----
 #define START_ID 0xFFFF
 #define END_ID 0xFFFF
 #define CLIENT_ID 0xFF
 #define LEN 0xFF
 #define DATA 0xFFF1
 #define ACK 0xFFF2
 #define REJECT 0xFFF3
 #define Error_OUTSEQ 0xFFF4
 #define Error_LEN 0xFFF5
 #define Error_END 0xFFF6
 #define Error_DUPLI 0xFFF7

 //modify the following value to other port (up to 65535)
 #define PORT 2000

 //----- Data Packet Format -----
 typedef struct DATA_PACKET{
     unsigned short start_id;
     unsigned char client_id;
     unsigned short data;
     unsigned char segno;
     unsigned char len;
     unsigned char payload[255];
     unsigned short end_id;
 } DATA_PACKET;

 //----- Received Response format ACK and REJECT packet -----
 typedef struct RECEIVED_PACKET{
     unsigned short start_id;
     unsigned char client_id;
     unsigned short REJorACK; //packet types
     unsigned short rejsub;
     unsigned char segno;
     unsigned short end_id;
 } RECEIVED_PACKET;

 int main(int argc, char *argv[]){

     struct sockaddr_in server_addr;
     socklen_t addr_len = sizeof server_addr;
     struct DATA_PACKET packet_set[5];
     struct RECEIVED_PACKET server_resp;
     int socketfd, correct_sequence = 0;

     //------ Create UDP socket ------
     if ((socketfd=socket(PF_INET, SOCK_DGRAM, 0)) < 0){
         perror("socket error: ");
         exit(1);
     }

     //------ Set IP and port------
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(PORT);
     server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
     memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);


     //----- create correct packets set (5 packets in a set) -----
     for(int i = 0; i < 5; i++){
         packet_set[i].start_id = START_ID;
         packet_set[i].client_id = CLIENT_ID;
         packet_set[i].data = DATA;
         packet_set[i].segno = i;
         packet_set[i].len = sizeof packet_set[i].payload;
         packet_set[i].end_id = END_ID;
     }
     // let the packet 3 be a copy of packet 2 to emulating Case-4 error (duplicated packet)
     packet_set[4] = packet_set[3];
     packet_set[3] = packet_set[2];
     //rest packets in the set will be correct and should be acknowledged


     //----- setup ack_timer -----
     struct pollfd pfds[0];
     pfds[0].fd = socketfd;
     pfds[0].events = POLLIN;

     int curr = 0;
     while(curr < 5) {
         //------ start sending packets ------
         printf("Client: Sending packet %d\n", packet_set[curr].segno);
         if(sendto(socketfd, &packet_set[curr], sizeof(DATA_PACKET),
             0, (struct sockaddr*)&server_addr, addr_len) < 0) {
             perror("send_error: ");
             exit(1);
         }

         //------ activate ack_timer and wait for event ------
         int event = poll(&pfds[0], 1, 3000), retry_counter = 1;
         while(event == 0){ //if no response
             if(retry_counter <= 3){ //reset ack_timer up to 3 times
             printf("Client: resending packet %d. attempt no.%d / 3\n", packet_set[curr].segno, retry_counter);
             if(sendto(socketfd, &packet_set[curr], sizeof(DATA_PACKET), 0,
                       (struct sockaddr *)&server_addr, addr_len) < 0) {
                 perror("send_error: ");
                 exit(1);
             }
             retry_counter++;
             event = poll(&pfds[0], 1, 3000);
           }
           else{ //resend reach 3 times, server no response
             printf("Server does not respond\n");
             close(socketfd);
             exit(0);
           }
         }

         //------ check for the after result of timer ------
         if(event < 0){ //polling error
             perror("poll_error: ");
             exit(1);
         }
         else{ //server responsed
             if(recvfrom(socketfd, &server_resp, sizeof(RECEIVED_PACKET), 0,
                         (struct sockaddr *)&server_addr, &addr_len) < 0){
                 perror("message_receive_error: ");
                 exit(1);
             }

             //------- check response type and print result ---------
             //server reponse REJECT
             if(server_resp.REJorACK == REJECT){
                 if(server_resp.rejsub == Error_OUTSEQ){ //CASE-1 error
                     printf("Server: Reject packet %d - Out of sequence. should receive packet %d\n\n",
                            server_resp.segno, correct_sequence+1);
                 }
                 else if(server_resp.rejsub == Error_LEN){ //CASE-2 error
                     printf("Server: Reject packet %d - Length mismatch.\n\n", server_resp.segno);
                     correct_sequence = server_resp.segno; // updating the correct sequence
                 }
                 else if(server_resp.rejsub == Error_END){ //CASE-3 error
                     printf("Server: Reject packet %d -  Missing end ID.\n\n", server_resp.segno);
                     correct_sequence = server_resp.segno; // updating the correct sequence
                 }
                 else if(server_resp.rejsub == Error_DUPLI){ //CASE-4 error
                     printf("Server: Reject packet %d - Duplicated packet. received packet %d again\n\n", server_resp.segno, server_resp.segno);
                 }
             }

             //server reponse ACK
             else if(server_resp.REJorACK == ACK){
                 server_resp.segno = server_resp.rejsub; //segment number field is identified in reject subcode field in ACK packet
                 printf("Server: ACK - Packet %d\n\n", server_resp.segno);
                 correct_sequence = server_resp.segno; //updating the correct sequence
             }
         }
         curr++;
     }
     close(socketfd);
 }
