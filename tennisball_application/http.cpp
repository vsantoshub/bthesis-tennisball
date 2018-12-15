//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <stdlib.h>         // Needed for exit()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <fcntl.h>          // Needed for file i/o stuff
#include <pthread.h>      // Needed for pthread_create() and pthread_exit()
#include <sys/stat.h>     // Needed for file i/o constants
#include <sys/types.h>    // Needed for sockets stuff
#include <netinet/in.h>   // Needed for sockets stuff
#include <sys/socket.h>   // Needed for sockets stuff
#include <arpa/inet.h>    // Needed for sockets stuff
#include <fcntl.h>        // Needed for sockets stuff
#include <netdb.h>        // Needed for sockets stuff
#include <iostream>
#include <unistd.h>  // problem solved! it compiles!
#include "http.h"

#include "application.h"
#include "server.h"
#include "debug.h"

//----- HTTP response messages ----------------------------------------------
#define OK_IMAGE  "HTTP/1.0 200 OK\r\nContent-Type:image/gif\r\n\r\n"
#define OK_TEXT   "HTTP/1.0 200 OK\r\nContent-Type:text/html\r\n\r\n"
#define NOTOK_404 "HTTP/1.0 404 Not Found\r\nContent-Type:text/html\r\n\r\n"
#define MESS_404  "<html><body><h1>FILE NOT FOUND</h1></body></html>"

//----- Defines -------------------------------------------------------------
#define  PORT_NUM            8080     // Port number for Web server
#define  BUF_SIZE            4096     // Buffer size (big enough for a GET)

//----- Function prototypes -------------------------------------------------
void *handle_get(void *in_arg);     // POSIX thread function to handle GET
//===== Main program ========================================================
static pthread_t http_thread;


int http_task(void)
{
    int                  server_s;             // Server socket descriptor
    struct sockaddr_in   server_addr;          // Server Internet address
    int                  client_s;             // Client socket descriptor
    struct sockaddr_in   client_addr;          // Client Internet address
    struct in_addr       client_ip_addr;       // Client IP address
    
    socklen_t            addr_len;             // Internet address length
    pthread_t            thread_id;            // Thread ID
    int                  retcode;              // Return code
    
    // Create a socket, fill-in address information, and then bind it
    server_s = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    retcode = bind(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (retcode < 0)
    {
        printf("*** ERROR - bind() failed \n");
        exit(-1);
    }
    
    // Set-up the listen
    listen(server_s, 100);
    
    // Main loop to accept connections and then spin-off thread to handle the GET
    printf(">>> weblite is running on port %d <<< \n", PORT_NUM);
    while(1)
    {
        addr_len = sizeof(client_addr);
        client_s = accept(server_s, (struct sockaddr *)&client_addr, &addr_len);
        if (client_s == -1)
        {
            printf("ERROR - Unable to create a socket \n");
            exit(1);
        }
        
        if (pthread_create(&thread_id, NULL, handle_get, (void *)client_s) != 0)
        {
            printf("ERROR - Unable to create a thread to handle the GET \n");
            exit(1);
        }
    }
    
    return(0);
}

//===========================================================================
//=  This is is the thread function to handle the GET                       =
//===========================================================================

void *handle_get(void *in_arg)
{
    int            client_s;             // Client socket descriptor
    char           in_buf[BUF_SIZE];     // Input buffer for GET request
    char           out_buf[BUF_SIZE];    // Output buffer for HTML response
    int            fh;                   // File handle
    int            buf_len;              // Buffer length for file reads
    char           command[BUF_SIZE];    // Command buffer
    char           file_name[BUF_SIZE];  // File name buffer
    int            retcode;              // Return code
    
    // Set client_s to in_arg
    client_s = (int) in_arg;
    
    // Receive the (presumed) GET request from the Web browser
    retcode = recv(client_s, in_buf, BUF_SIZE, 0);
    
    // If the recv() return code is bad then bail-out (see note #3)
    if (retcode <= 0)
    {
        printf("ERROR - Receive failed --- probably due to dropped connection \n");
        
        close(client_s);
        pthread_exit(NULL);
    }
    
    // Parse out the command from the (presumed) GET request and filename
    sscanf(in_buf, "%s %s \n", command, file_name);
    
    // Check if command really is a GET, if not then bail-out
    if (strcmp(command, "GET") != 0)
    {
        printf("ERROR - Not a GET --- received command = '%s' \n", command);
        
        close(client_s);
        pthread_exit(NULL);
    }
    
    // It must be a GET... open the requested file
    //  - Start at 2nd char to get rid of leading "\"
    
    fh = open(&file_name[1], O_RDONLY, S_IREAD | S_IWRITE);
    
    // If file does not exist, then return a 404 and bail-out
    if (fh == -1)
    {
        printf("File '%s' not found --- sending an HTTP 404 \n", &file_name[1]);
        strcpy(out_buf, NOTOK_404);
        send(client_s, out_buf, strlen(out_buf), 0);
        strcpy(out_buf, MESS_404);
        send(client_s, out_buf, strlen(out_buf), 0);
        
        close(client_s);
        pthread_exit(NULL);
    }
    
    // Check that filename does not start with a ".", "/", "\", or have a ":" in
    // the second position indicating a disk identifier (e.g., "c:").
    //  - This is a security check to prevent grabbing any file on the server
    if ((file_name[1] == '.') || (file_name[1] == '/') ||
        (file_name[1] == '\\') || (file_name[2] == ':'))
    {
        printf("SECURITY VIOLATION --- trying to read '%s' \n", &file_name[1]);
        close(fh);
        
        close(client_s);
        pthread_exit(NULL);
    }
    
    // Generate and send the response
    // printf("Sending file '%s' \n", &file_name[1]);
    if (strstr(file_name, ".gif") != NULL)
        strcpy(out_buf, OK_IMAGE);
    else
        strcpy(out_buf, OK_TEXT);
    send(client_s, out_buf, strlen(out_buf), 0);
    while(1)
    {
        buf_len = read(fh, out_buf, BUF_SIZE);
        if (buf_len == 0) break;
        send(client_s, out_buf, buf_len, 0);
    }
    
    // Close the file, close the client socket, and end the thread
    close(fh);
    
    close(client_s);
    pthread_exit(NULL);
}

int http_start(void) {

    int ret;

    if ((ret = pthread_create(&http_thread, NULL, 
                    (void * (*)(void *))http_task, 
                    (void *)NULL)) < 0) {
        DBG(DBG_ERROR, "pthread_create()");
        return ret;
    }

    return 0;

}

int http_stop(void)
{

    pthread_cancel(http_thread);
    pthread_join(http_thread, NULL);
    return 0;
}
