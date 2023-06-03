#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>

#define PORT_NUMBER "9000"
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define PACKET_SIZE 128

static int server_socket;
static int client_socket;
static int fd;
char * tx_buf, * rx_buf;

typedef enum{
    DEFAULT_MODE = 0,
    DAEMON_MODE
}prog_mode_t;

static void free_all(void){
    free(tx_buf);
    free(rx_buf);
    close(fd);
    close(client_socket);
    close(server_socket);
    closelog();
}

static void signal_handler(int sig_no){
    if(sig_no == SIGINT || sig_no == SIGTERM){
        free_all();
        remove(FILE_PATH);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]){
    prog_mode_t mode = DEFAULT_MODE;

    openlog(NULL, 0, LOG_USER);

    // check args
    if(argc == 2){
        if(!strcmp("-d", argv[1])){
            mode = DAEMON_MODE;
        }
    }

    // init signals
    if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		return -1;
	}										

	if(signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		return -1;
	}

    sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

    // create the server socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        syslog(LOG_ERR, "Error - creating the server socket\n");
        closelog();
        return -1;
    }

    int reuse_addr = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) < 0) 
	{
		syslog(LOG_ERR, "Error - setsockopt\n");
		close(server_socket);
		closelog();
		return -1;
	}

    // define the server address
	struct addrinfo server_address;
    memset(&server_address, 0, sizeof(struct addrinfo));
	server_address.ai_family = AF_INET;
	server_address.ai_socktype = SOCK_STREAM;
	server_address.ai_flags = AI_PASSIVE;
  
    struct addrinfo * server_address_info;
    if(getaddrinfo(NULL, PORT_NUMBER, &server_address, &server_address_info) != 0)
	{
		syslog(LOG_ERR, "Error - getting address info\n");
		close(server_socket);
		closelog();
		return -1;
	}

    // bind the socket to our specified IP and port
	if(bind(server_socket, server_address_info->ai_addr, server_address_info->ai_addrlen) < 0){
        syslog(LOG_ERR, "Error - binding\n");
        close(server_socket);
        closelog();
        return -1;
    }

    freeaddrinfo(server_address_info);
    if(mode == DAEMON_MODE){
        int pid = fork();

        if(pid == 0){
            // Child process
            int sid = setsid();
            if(sid < 0){
                syslog(LOG_ERR, "Error - setsid\n");
                exit(EXIT_FAILURE);
            }
            if(chdir("/") < 0){
                syslog(LOG_ERR, "Error - chdir");
                close(server_socket);
                closelog();
                exit(EXIT_FAILURE);
                dup(0);
                dup(0);
            }
            open("/dev/null", O_RDWR);
            perror("open");
        }
        else if(pid > 0){
            // Parent process
            syslog(LOG_DEBUG, "Info - Daemon created\n");
            exit(EXIT_SUCCESS);
        }
        else{
            // Parent process, child couldn't be created
            syslog(LOG_ERR, "Error - Fork\n");
        }

    }

    if(listen(server_socket, 6) < 0){
        syslog(LOG_ERR, "Error - Listen\n");
        close(server_socket);
        closelog();
        return -1;
    }  

    rx_buf = malloc (sizeof(char) * PACKET_SIZE);
    if(rx_buf == NULL){
        syslog(LOG_ERR, "Error - malloc rx buffer\n");
        close(server_socket);
        closelog();
        return -1;
    }

    fd = open(FILE_PATH, O_RDWR | O_APPEND | O_CREAT, 0744);
    if(fd < 0){
        syslog(LOG_ERR, "Error - open file\n");
        free(rx_buf);
        close(server_socket);
        closelog();
        return -1;
    }

    struct sockaddr_in client_address;
    socklen_t client_addr_size;
    client_addr_size = sizeof(client_address);
    size_t buffer_realloc_count = 0;
    size_t total_rx_len = 0;
    while(1){
        
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_addr_size);
        if (client_socket < 0){
            syslog(LOG_ERR, "Error - accept client\n");
            free(rx_buf);
            close(server_socket);
            closelog();
            return -1;
        }
        else{
            char *client_ip_address = inet_ntoa(client_address.sin_addr);
            syslog(LOG_DEBUG, "Accepted connection from %s\n", client_ip_address);
        }
        size_t rx_buffer_index = 0;
        int byte_len;
        do{
            byte_len = recv(client_socket, (rx_buf+rx_buffer_index), PACKET_SIZE - 1, 0);
            if(byte_len < 0){
                syslog(LOG_ERR, "Error - recv\n");
                free(rx_buf);
                close(server_socket);
                close(client_socket);
                closelog();
                return -1;
            }
            else{
                rx_buffer_index += byte_len;
                total_rx_len += byte_len;

                buffer_realloc_count++;
				rx_buf = realloc(rx_buf, (buffer_realloc_count) * (PACKET_SIZE) * (sizeof(char)));		
				if(rx_buf == NULL)
				{
					syslog(LOG_ERR, "Error - reallocating buffer\n");
                    free(rx_buf);
                    close(server_socket);
                    close(client_socket);
                    closelog();
                    return -1;
				}
            }

        }while(strchr(rx_buf, '\n') == NULL);
        rx_buf[rx_buffer_index] = '\0';

        if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
            syslog(LOG_ERR, "Error - sigprocmask\n");
            free(rx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
        }

        if(write(fd, rx_buf, rx_buffer_index) < 0){
            syslog(LOG_ERR, "Error - write\n");
            free(rx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
        }

        if(sigprocmask(SIG_UNBLOCK, &mask, NULL) < 0){
            syslog(LOG_ERR, "Error - sigprocmask\n");
            free(rx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
        }

        tx_buf = malloc(sizeof(char) * total_rx_len);	  
		if(tx_buf==NULL)
		{
            syslog(LOG_ERR, "Error - malloc of tx_buf\n");
            free(rx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
		}

        lseek(fd, 0, SEEK_SET);
		
        int bytes_read = read(fd, tx_buf, total_rx_len);
		if(bytes_read == 1)
		{
            syslog(LOG_ERR, "Error - read from fd\n");
            free(rx_buf);
            free(tx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
		}

        int tx_bytes_sent = send(client_socket, tx_buf, bytes_read,0);
        if(tx_bytes_sent == -1){
            syslog(LOG_ERR, "Error - send\n");
            free(rx_buf);
            free(tx_buf);
            close(server_socket);
            close(client_socket);
            closelog();
            return -1;
        }

        if(tx_bytes_sent == bytes_read){
            syslog(LOG_DEBUG,"Info - The contents written succefully\n");
        }

    }
        
    free(rx_buf);
    free(tx_buf);
    close(server_socket);
    close(client_socket);
    closelog();

    return 0;
}