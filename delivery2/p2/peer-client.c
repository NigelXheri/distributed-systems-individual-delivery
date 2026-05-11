/*
MECHANISM: P2P
FILE: simple-client-chat.c
DESCRIPTION: This client implements communication between peers through the UDP protocol. It uses select to multiplex the reads between the keyboard and the p2p socket.
*/

#include <time.h>      // ADDED Nigel Xherimeja
#include "protocol.h"  // ADDED Nigel Xherimeja


#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define MAX_LINE_SIZE 500
#define MAX_MSG_SIZE 500
#define MAX_NICK_SIZE 10

char *nick; // To send it in each message


// ADDED Nigel Xherimeja
char broker_ip[25];
int broker_port_g;
int pong_interval;


// These functions are implemented after main()
void read_and_process_keyboard(int udp_socket);
void receive_and_show_message(int udp_socket);
int get_max(int a, int b);



// ADDED Nigel Xherimeja
void log_pong_sent(const char *nick) {
    if (getenv("DEBUG_PONG")) {
        time_t now = time(NULL);
        printf("\n[%ld] [PONG SENT] nick: %s\n%s> ", (long)now, nick, nick);
        fflush(stdout);
    }
}

// ADDED Nigel Xherimeja
// Opens a TCP connection to broker, sends a command, reads response.
// For JOIN_CMD, pass the p2p_port. For QUERY_CMD, fills *result on CMD_OK.
char send_broker_cmd(char cmd, char *target_nick, int p2p_port, struct sockaddr_in *result) {
    int sock = CreateTCPDataSocket();
    Connect(sock, broker_ip, broker_port_g);

    short nick_len = (short)strlen(target_nick);
    SendByte(sock, cmd);
    SendShort(sock, nick_len);
    SendString(sock, target_nick);

    if (cmd == JOIN_CMD)
        SendShort(sock, (short)p2p_port);

    char response = ReadByte(sock);

    if (response == CMD_OK && cmd == QUERY_CMD && result != NULL)
        ReadEndpoint(sock, result);

    Close(sock);
    return response;
}

// ADDED Nigel Xherimeja
void send_pong() {
    char response = send_broker_cmd(PONG_CMD, nick, 0, NULL);
    if (response == CMD_OK)
        log_pong_sent(nick);
}


int main(int argc, char *argv[])
{
    int udp_socket;
    int port; // For the socket between peers
    struct sockaddr_in udp_addr;
    int keyboard = 0; // Descriptor of the standard input

    // For the select
    fd_set listen;
    int max;
    int result;

    // Checking arguments
    // ADDED Nigel Xherimeja
    if (argc != 6) {
        printf("Usage: %s <peer_port> <nick> <broker_ip> <broker_port> <pong_interval>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);
    nick = argv[2];
    if (strlen(nick) >= MAX_NICK_SIZE - 1)
        nick[MAX_NICK_SIZE - 1] = 0;
    strncpy(broker_ip, argv[3], 24);
    broker_port_g = atoi(argv[4]);  // ADDED Nigel Xherimeja
    pong_interval  = atoi(argv[5]); // ADDED Nigel Xherimeja


    // Socket initialization
    // We create the UDP socket in the port that is passed to it by command line
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1)
    {
        perror("When creating the UDP socket");
        exit(1);
    }
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_addr.sin_port = htons(port);
    result = bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr_in));
    if (result == -1)
    {
        perror("When doing bind");
        exit(1);
    }

    // ADDED Nigel Xherimeja
    char join_resp = send_broker_cmd(JOIN_CMD, nick, port, NULL);
    if (join_resp != CMD_OK) {
        printf("Error: could not register with broker (nick may already be taken)\n");
        exit(1);
    }
    printf("Registered with broker as %s\n", nick);


    printf("Use the /CHAT <nick> command to indicate the destination of your messages\n");  // ADDED Nigel Xherimeja
    printf("The text you write later will be sent to that contact\n\n");
    printf("At any time you can put /CHAT <nick> again\n"); // ADDED Nigel Xherimeja
    printf("to switch to a new contact.\n\n");

    // And we enter the waiting loop
    printf("%s>", nick);

    struct timeval tv;      // ADDED Nigel Xherimeja
    tv.tv_sec  = pong_interval;     // ADDED Nigel Xherimeja
    tv.tv_usec = 0;                 // ADDED Nigel Xherimeja


    while (1)
    {
        // Print a prompt to invite to write
        fflush(stdout); // To get it out now, since it doesn't end with \n

        // TODO
        // Select initialization
        // ADDED Nigel Xherimeja
        FD_ZERO(&listen);
        FD_SET(udp_socket, &listen);
        FD_SET(keyboard, &listen);
        max = get_max(udp_socket, keyboard);

        
        // TODO
        // Wait in select for the user to write something, or for something to arrive
        // over the UDP socket
        result = select(max + 1, &listen, NULL, NULL, &tv);
        if (result == 0) {
            tv.tv_sec  = pong_interval;     // ADDED Nigel Xherimeja
            tv.tv_usec = 0;                 // ADDED Nigel Xherimeja
            send_pong();
            continue;
        }

        // When exiting, something has happened
        if (FD_ISSET(udp_socket, &listen))
        {
            receive_and_show_message(udp_socket);
        }

        if (FD_ISSET(keyboard, &listen))
        {
            read_and_process_keyboard(udp_socket);
        }
    }
}

void receive_and_show_message(int udp_socket)
{
    char buff[MAX_MSG_SIZE];
    int received;

    received = recvfrom(udp_socket, buff, MAX_MSG_SIZE, 0, NULL, NULL);
    if (received == -1)
        return; // We silently ignore errors
    buff[received] = 0;
    printf("\n\t\t%s\n", buff);
    printf("\n%s> ", nick);
}

void read_and_process_keyboard(int udp_socket)
{
    char line[MAX_LINE_SIZE];
    char cmd[MAX_LINE_SIZE];
    static char destination_ip[25] = "Unassigned";
    static int destination_port = 0;
    static struct sockaddr_in target_dir;
    char *message_to_send;
    int i;
    static char current_nick[MAX_NICK_SIZE] = {0};     // ADDED Nigel Xherimeja

    // Read the line
    // ADDED Nigel Xherimeja
    if (fgets(line, MAX_LINE_SIZE - MAX_NICK_SIZE, stdin) == NULL) {
        send_broker_cmd(LEAVE_CMD, nick, 0, NULL);
        printf("\nGoodbye!\n");
        exit(0);
    }

    // Remove the carriage return or spaces at the end
    for (i = strlen(line) - 1; i >= 0; i--)
    {
        if (isspace(line[i]))
            line[i] = 0;
        else
            break;
    }

    // See if it's a command
    if (line[0] == '/')
    {
        // If begins with /, it is a command
        // The easiest thing is to read its contents with sscanf
        sscanf(line, "%s", cmd);

        // TODO
        // Depending on the value found in cmd, we perform the appropriate action
        // ADDED Nigel Xherimeja
        if (strcmp(cmd, "/CHAT") == 0) {
            char target[MAX_NICK_SIZE];
            sscanf(line, "%s %s", cmd, target);
            struct sockaddr_in ep;
            char resp = send_broker_cmd(QUERY_CMD, target, 0, &ep);
            if (resp != CMD_OK) {
                printf("Error: user '%s' not found\n", target);
            } else {
                ep.sin_family = AF_INET;
                memcpy(&target_dir, &ep, sizeof(struct sockaddr_in));
                strcpy(destination_ip, inet_ntoa(ep.sin_addr));
                destination_port = ntohs(ep.sin_port);
                strcpy(current_nick, target); // ADDED Nigel Xherimeja
                printf("Now chatting with %s (%s:%d)\n", target, destination_ip, destination_port);
            }
        } else if (strcmp(cmd, "/QUIT") == 0) {
            send_broker_cmd(LEAVE_CMD, nick, 0, NULL);
            printf("Goodbye!\n");
            exit(0);
        } else {
            printf("Unknown command: %s\n", cmd);
        }

        // Get a new prompt
        // ADDED Nigel Xherimeja
        printf("%s>", nick);
    }

    else // If the line does not start with /, it is a message to send to the peer
    {
        // You have to send it to the ip and ports previously assigned,
        // but first we check if they were actually assigned
        // before these values
        if (strcmp(destination_ip, "Unassigned") == 0)
        {
            printf("ERROR: before sending message you must use the command\n");
            // ADDED Nigel Xherimeja
            printf("/CHAT <nick>\n");
            printf("%s>", nick);
            return;
        }
        // If everything goes well, we send the message,
        // preceded by the user's nick

        // TODO
        // Create a buffer with the message to send and send it
        // ADDED Nigel Xherimeja
        struct sockaddr_in ep;
        char resp = send_broker_cmd(QUERY_CMD, current_nick, 0, &ep);
        if (resp != CMD_OK) {
            printf("Error: user '%s' not found\n", current_nick);
            printf("You need to USE: /CHAT <nick> again because %s has been evicted from the server registry\n", current_nick);
            strcpy(destination_ip, "Unassigned");
            destination_port = 0;
            memset(current_nick, '\0', sizeof(current_nick));
            printf("%s>", nick);
        } 
        else {
            char message[MAX_MSG_SIZE + strlen(nick) + 3];
            snprintf(message, sizeof(message), "%s: %s", nick, line);
            sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)&target_dir, sizeof(struct sockaddr_in));
            printf("%s>", nick);
        }
    }
}

int get_max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}