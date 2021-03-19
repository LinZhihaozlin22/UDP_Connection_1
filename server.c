/*
 Student ID: 1607869
 Name: Zhihao Lin

 Assignment 1 - server source code
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

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

//----- ACK Packet Format -----
typedef struct ACK_PACKET{
    unsigned short start_id;
    unsigned char client_id;
    unsigned short ack;
    unsigned char segno;
    unsigned short end_id;
} ACK_PACKET;

//----- REJECT PACKET FORMAT -----
typedef struct REJ_PACKET{
    unsigned short start_id;
    unsigned char client_id;
    unsigned short rej;
    unsigned short rejsub;
    unsigned char segno;
    unsigned short end_id;
} REJ_PACKET;

int main(int argc, char *argv[]){

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof client_addr;
    struct DATA_PACKET client_msg;
    struct ACK_PACKET ACK_P;
    struct REJ_PACKET REJ_P;
    int socketfd, expect_seq = 0, received_packet = 0;

    //------ Create UDP socket ------
    if((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket error: ");
        exit(1);
    }

    //------ Set IP and port------
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);


    //------ Bind socket to host ------
    if(bind(socketfd, (struct sockaddr *)&server_addr, sizeof server_addr) < 0){
        perror("binding error: ");
        exit(1);
    }
    printf("Listening for client messages...\n\n");

    
    while(1){
        //------ Receiving client message ------
        if (recvfrom(socketfd, &client_msg, sizeof(DATA_PACKET), 0,
                     (struct sockaddr*)&client_addr, &addr_len) < 0){
            perror("message_receive_error: ");
            exit(1);
        } else{
            // msg received, setup reject message for detecting errors
            REJ_P.start_id = START_ID;
            REJ_P.client_id = client_msg.client_id;
            REJ_P.rej = REJECT;
            REJ_P.segno = client_msg.segno;
            REJ_P.end_id = END_ID;
        }

        //-------- check message and reponse --------
        if(expect_seq - 1 == client_msg.segno){ //Case-4 error
            REJ_P.rejsub = Error_DUPLI;
            if (sendto(socketfd, &REJ_P, sizeof(REJ_PACKET), 0,
                       (struct sockaddr*)&client_addr, addr_len) < 0){
                perror("send_error: ");
                exit(1);
            }
            printf("Reject - Packet %d (Duplicated packet: received packet %d again)\n", client_msg.segno, client_msg.segno);
        }

        else if(expect_seq != client_msg.segno){ //Case-1 error
            REJ_P.rejsub = Error_OUTSEQ;
            if (sendto(socketfd, &REJ_P, sizeof(REJ_PACKET), 0,
                       (struct sockaddr*)&client_addr, addr_len) < 0){
                perror("send_error: ");
                exit(1);
            }
            printf("Reject - Packet %d (Out_of_sequence: should receive packet %d)\n", client_msg.segno, expect_seq);
        }

        else if(client_msg.len != sizeof(client_msg.payload)){ //Case-2 error
            REJ_P.rejsub = Error_LEN;
            if (sendto(socketfd, &REJ_P, sizeof(REJ_PACKET), 0,
                       (struct sockaddr*)&client_addr, addr_len) < 0){
                perror("send_error: ");
                exit(1);
            }
            printf("Reject - Packet %d (Length_error: Error_length-%d Correct_length-%lu)\n", client_msg.segno, client_msg.len, sizeof(client_msg.payload));
            expect_seq = client_msg.segno + 1; // update expected sequence number
        }

        else if(client_msg.end_id != END_ID){ //Case-3 error
            REJ_P.rejsub = Error_END;
            if (sendto(socketfd, &REJ_P, sizeof(REJ_PACKET), 0,
                       (struct sockaddr*)&client_addr, addr_len) < 0){
                perror("send_error: ");
                exit(1);
            }
            printf("Reject - Packet %d (Missing_end_id_error: Last bytes of packet-%d)\n", client_msg.segno, client_msg.end_id);
            expect_seq = client_msg.segno + 1; //update expected sequence number
        }

        else{ //No error detected, response ACK
            ACK_P.start_id = START_ID;
            ACK_P.client_id = client_msg.client_id;
            ACK_P.ack = ACK;
            ACK_P.segno = client_msg.segno;
            ACK_P.end_id = END_ID;

            if (sendto(socketfd, &ACK_P, sizeof(ACK_PACKET), 0,
                       (struct sockaddr*)&client_addr, addr_len) < 0){
                perror("send_error: ");
                exit(1);
            }
            printf("ACK - Packet %d\n", client_msg.segno);
            expect_seq = client_msg.segno + 1; // update expected sequence number
        }

        /* received_packet is used for test, since the client will send 5 packets as a set,
         updating after every 5 packets to keep the correct expected sequence */
        received_packet++;
        if(received_packet == 5){
            printf("------------- End of one packets set -------------\n\n");
            received_packet = 0;
            expect_seq = 0;
        }
    }
    /* To keep server running, close() will not be used.
     you may need to mannually stop the program */
    return 0;
}
