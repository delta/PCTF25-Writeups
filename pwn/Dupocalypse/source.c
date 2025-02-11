# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>


        
void pwn(int a, int b, int c)
{       
        if(c == 0xcafebabe)
        {
                FILE *file;
                file = fopen("app/flag.txt", "r");
                if (file) {
                        char flag[100];
                        fgets(flag, sizeof(flag), file);
                        write(1, flag, strlen(flag));
                        fclose(file);
                } else {
                       write(1, "Contact admin\n", 14);
                }
        }
}

void whereami(void * addr, int fd)
{
        char buffer[60];
        int len = snprintf(buffer, sizeof(buffer), "The stack has spoken:%p\nThe rest is up to you!\n", addr);
        write(fd, buffer, len);
}

int getinput(int fd)
{
        write(fd,"Greetings, challenger. Let’s begin!\n", 39);
        char buf[0x100];
        whereami(buf,fd);
        memset(buf, 0, 0x100);
        write(fd,"Your move, challenger. What’s your input?\n", 45);
        read(fd, buf, 0x118);
        write(fd,"Got it. Let’s see what unfolds...\n", 37);
        return 0;
}


void error(const char *msg)
{
        perror(msg);
        exit(1);
}

void dupx()
{
        dup2(1,1);
}

int main(int argc, char *argv[]) {

        int PORT;

        char *port_env = getenv("PORT");
        if (port_env != NULL) {
                PORT = atoi(port_env);
        }
        else {
                exit(1);
        }

        int server_fd, client_fd;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
                error("Socket creation failed");
        }

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                error("Setsockopt failed");
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;  
        server_addr.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                error("Bind failed");
        }

        if (listen(server_fd, 1) < 0) {
                error("Listen failed");
        }

        printf("Server is listening on port %d...\n", PORT);
        

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
                error("Accept failed");
        }

        write(1, "Accepted a connection...\n", 26);

        getinput(client_fd);

        close(server_fd);
        close(client_fd);
        write(1, "Server shut down.\n", 18);

        return 0;
}