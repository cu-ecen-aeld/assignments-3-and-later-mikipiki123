#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


#define PORT 9000
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFSIZE 1024

int server_fd, new_socket = -1;

int daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS); // parent exiting
    }

    // child process
    if (setsid() < 0) {
        syslog(LOG_ERR, "Failed to create a new session: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        dup2(dev_null, STDOUT_FILENO);
        dup2(dev_null, STDERR_FILENO);
        if (dev_null > 2) close(dev_null);
    }

    return 0;
}

void signal_handler(int sig) {
    syslog(LOG_INFO, "Caught signal, exiting");
    remove(FILEPATH);

    printf("Caught signal, exiting\n");

    if (new_socket != -1) {
        close(new_socket);
    }

    if (server_fd != -1) {
        close(server_fd);
    }


    closelog();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    //todo daemon mode 5

    // parse arguments to check for -d option
    int daemon_mode = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            daemon_mode = 1;
        }
    }

    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    //open log service
    openlog("aesdsocket", LOG_PID | LOG_PERROR, LOG_USER);

    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFSIZE];

    //declaring socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation FAILED.\n");
        close(server_fd);
        return -1;
    }

    //to prevent "address in use" issue
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind socket to IP and PORT
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0) {
        perror("bind FAILED.\n");
        close(server_fd);
        return -1;
    }

    // checking for Daemon after binding if -d option was specified
    if (daemon_mode) {
        if (daemonize() != 0) {
            close(server_fd);
            return -1;
        }
        syslog(LOG_INFO, "Running in daemon mode");
    }



    //listen to connection
    if (listen(server_fd, 5) != 0) {
        perror("listen FAILED.\n");
        close(server_fd);
        return -1;
    }

    printf("Waiting for a client connection...\n");

    //accept connection loop
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept FAILED.\n");
            close(server_fd);
            break;
        }



        // Convert IP to human-readable form and send log
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), ip_str, sizeof(ip_str));
        printf("Client connected from %s:%d\n", ip_str, ntohs(address.sin_port));
        syslog(LOG_INFO, "Accepted connection from %s", ip_str);

        // Open file for reading and writing; create if it doesn't exist; set permissions to 0644
        int fd = open(FILEPATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            perror("open failed");
            break;
        }

        //receive and write to FILEPATH
        while ((valread = recv(new_socket, buffer, BUFSIZE, 0)) > 0) {
            printf("Received!\n");
            write(fd, buffer, valread);

            // check for end of packet based on '\n' as the delimiter
            if (buffer[valread - 1] == '\n') {
                break;
            }
        }

        close(fd);

        fd = open(FILEPATH, O_RDONLY);
        if (fd < 0) {
            perror("open failed");
            break;
        }

        // read the full content of the file and send it back to the client
        while ((valread = read(fd, buffer, BUFSIZE)) > 0) {
            printf("Sent!\n");
            send(new_socket, buffer, valread, 0);
        }

        close(fd);
        close(new_socket);
        printf("Closed connection from %s\n", ip_str);
        syslog(LOG_INFO, "Closed connection from %s", ip_str);

    //todo create handlers for SIGINT and SIGTERM
    //todo when interrupt - close connections gracefully and delete FILEPATH

    }

    closelog();
    close(server_fd);
    return 0;
}
