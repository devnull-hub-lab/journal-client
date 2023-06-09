#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 2628
#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    
    short flag_r = 0, flag_n = 0;
    int flag_n_num = 0; //unlimited entries
    int flags_opt = 0;


    if (argc < 2 || argc > 5) {
        fprintf(stderr, "Usage: %s [-r] [-n <num>] user@host\n\n", argv[0]);
        fprintf(stderr, "-r: Reverse Journal Search. Searches in the reverse order in which journal entries were entered: most recent first.\n");
        fprintf(stderr, "-n <num>: Number of journal entries to show.\n\n");
        exit(1);
    }

    int userHostIndex = 1;

    while ((flags_opt = getopt(argc, argv, "rn:")) != -1) {
        switch (flags_opt) {
            case 'r':
                flag_r = 1;
                break;
            case 'n':
                flag_n = 1;
                flag_n_num = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Invalid option: -%c\n", optopt);
                exit(1);
        }
    }

    if (flag_n && flag_n_num == 0) {
    fprintf(stderr, "You must specify a number for -n <num> \n");
    exit(1);
}

    userHostIndex = optind;

    char user[32] = ""; //limit unix system
    char host[255] = ""; //limit IETF spec
    char flags[50] = "";
    char buffer[MAX_BUFFER_SIZE] = "";
    sscanf(argv[userHostIndex], "%[^@]@%s", user, host);

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error on open socket");
        exit(1);
    }

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error, host '%s' not found\n", host);
        exit(1);
    }

    //set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on estabilishing connection");
        exit(1);
    }

    if (flag_r) {
        strcat(flags, "r");
    }
    if (flag_n) {
        strcat(flags, "n");
    }

    //sending param to server
    snprintf(buffer, sizeof(buffer), "%s@%s%d", user, flags, flag_n_num);

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error on sending data to server");
        exit(1);
    }

    // Reading and printing server data
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead;
    while ((bytesRead = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    if (bytesRead < 0) {
        perror("Error reading server data");
        exit(1);
    }

    if (strstr(buffer, "Journal not found") != NULL) {
        close(sockfd);
        return 1;  // Set the exit code to indicate an error
    }

    close(sockfd);

    return 0;
}
