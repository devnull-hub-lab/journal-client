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
    int flags_opt = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s [-r] user@host\n", argv[0]);
        fprintf(stderr, "See man for more info\n");
        exit(1);
    }

    int userHostIndex = 1;

    while ((flags_opt = getopt(argc, argv, "rst")) != -1) {
        switch (flags_opt) {
            case 'r':
                flag_r = 1;
                break;
            //case 's':
            //    flag_n = 1;
            //    break;
            default:
                fprintf(stderr, "Invalid option: -%c\n", optopt);
                exit(1);
        }
    }

    if (flag_r || flag_n)
        userHostIndex = 2;

    char user[MAX_BUFFER_SIZE] = "", host[MAX_BUFFER_SIZE] = "", flags[MAX_BUFFER_SIZE] = "";
    char buffer[MAX_BUFFER_SIZE] = "";
    sscanf(argv[userHostIndex], "%[^@]@%s", user, host);

    int sockfd, n;
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
    //if (flag_n) {
    //    strcat(flags, "s");
    //}

    //sending param to server
    snprintf(buffer, sizeof(buffer), "%s@%s", user, flags);
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
