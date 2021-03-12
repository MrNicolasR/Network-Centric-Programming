/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Nicolas RUbert
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
*/

/* 
 * Include headers 
*/
#include "csapp.h"

/*
 * Function prototypes
*/
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void server(int port);
void process(int connfd, struct sockaddr_in *clientaddr);
int parse_uri(char *uri, char *target_addr, char *path, int *port);
void client(char *host, int port, int connfd, rio_t rio1, char buf[], struct sockaddr_in *clientaddr, char uri[]);


/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    // Get port number from arguements
    int port = atoi(argv[1]);

    // Start message
    printf("Proxy Started...\n");
    printf("Awaiting connections...\n");
    // Start Server
    server(port);

    // Exit
    exit(0);
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}

/* Implemented Functions */

/*
* server function
* It will listen for incoming connections from clients and start processing thier requests.
* (Modified from the CSAAP Computer Systems book. Chapter 11)
*/
void server(int port)
{
    int listenfd, connfd;          // file descriptors
    socklen_t clientlen;           // Client's address size
    struct sockaddr_in clientaddr; // Store client's address

    listenfd = open_listenfd(port); // Listen for connections

    // Wait/Accept client connection
    while (1)
    {
        clientlen = sizeof(clientaddr);                           // Client's address size
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // Accept the connection from the client

        // Debugging
        printf("Accepted Connection\n"); // Print Message

        // Run Process fucntion
        process(connfd, &clientaddr); // Process Request

        // Close file descriptor
        close(connfd);
    }
}

/* 
* process fucntion 
*/
void process(int connfd, struct sockaddr_in *clientaddr)
{
    // Variables
    char buf[MAXLINE];      // Buffer
    char method[MAXLINE];   // GET method
    char uri[MAXLINE];      // Uri
    char version[MAXLINE];  // Version
    char hostname[MAXLINE]; // Hostname
    char pathname[MAXLINE]; // Pathname
    int port;               // Port number
    char *errorM = "HTTP/1.1 502 Bad Gateway\r\n";

    rio_t rio; // Rio (where we read/write to connfd)

    // Read request line and headers
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);             // Read to buf
    sscanf(buf, "%s %s %s", method, uri, version); // Read headers

    // Make sure the Mehtod is GET
    if (strcmp(method, "GET") != 0)
    {
        // Send error Message to Client
        Rio_writen(connfd, (void *)errorM, strlen(errorM));
        printf("Process Error: Method is not GET\n"); // Print Error Message to server
        return;
    }

    // If there is an error with parsing the request
    if (parse_uri(uri, hostname, pathname, &port) <= -1)
    {
        // Print Error
        printf("Parsing Error: Unable to parse uri\n");
        return;
    }

    // Debugging
    printf("Request hostname: %s, pathname: %s, port: %d\n", hostname, pathname, port);
    // Run Client side
    client(hostname, port, connfd, rio, buf, clientaddr, uri); // Step 3: Use proxy as a client to the end server.
}

/*
* parse_uri function 
* From the GET Request parse through the uri (url of the website). 
* The program will get the hostname, path and the port. The program 
* will return 0 if successful or -1 and let the Process function
* deal with error handling.
* (Modified from the CSAAP Computer Systems book. Chapter 11) 
*/
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{

    // Variables
    char *hostbegin; // Hostname
    char *hostend;   // End characters
    char *path;      // Pathname
    int len;         // Length of the uri

    // Make sure its request is a http site
    if (strncasecmp(uri, "http://", 7) != 0)
    {
        hostname[0] = '\0';
        return -1;
    }

    // Get Hostname from request Example" "yahoo.com"
    hostbegin = uri + 7;
    // Extract the final characters
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    // Copy to hostname
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    *port = 80; // Defualt portnumber
    // Get port number from request
    if (*hostend == ':')
    {
        // Convert character into number
        *port = atoi(hostend + 1);
    }

    // Extract the path from request Example: "/news"
    path = strchr(hostbegin, '/');
    if (path == NULL)
    {
        pathname[0] = '\0';
    }
    else
    {
        path++;
        strcpy(pathname, path);
    }

    return 0;
}

/*
* Client function
* Connects to the proxy. Receives the response from the proxy.
* Writes back to the browser. Starts the logging process aswell.
* (Modified from the CSAAP Computer Systems book. Chapter 11)
*/
void client(char *host, int port, int connfd, rio_t rio1, char buf[], struct sockaddr_in *clientaddr, char uri[])
{
     // Variables
    int clientfd; // Client socket
    int size = 0; // The size of the content that server returns in bytes
    ssize_t linesize; // Size of the line 
    char bufA[MAXLINE]; // Buffer 
    char bufB[MAXLINE]; // Buffer
    char requestblock[MAXLINE]; // Request block
    char logstring[MAXLINE]; // Logstring (for writing to log file)
    rio_t rio2; // Rio
    
    // Create file pointer (log file)
    FILE *file;

    requestblock[0] = '\0'; // initialization Block
    // Write contents of buffer into request block
    strcat(requestblock, buf);
    // Read Contents of rio1 into buffer 1
    Rio_readlineb(&rio1, bufA, MAXLINE);

    // Make sure connection is valid
    while (strcmp(bufA, "\r\n"))
    {
        if (strstr(bufA, "Connection:") == NULL && strstr(bufA, "Proxy-Connection:") == NULL)
        {
            strcat(requestblock, bufA);
        }
        Rio_readlineb(&rio1, bufA, MAXLINE);
    }

     // If the connection between proxy-server and client ends
    strcat(requestblock, "Connection: close\r\nProxy-Connection: close\r\n\r\n");

    // Receive the response from the server 
    if ((clientfd = open_clientfd(host, port)) < 0)
    {
        printf("Client Error: Unable to connect to proxy\n");
    }

    // Write the response into the  request block
    Rio_writen(clientfd, requestblock, strlen(requestblock));

    // Server Response
    Rio_readinitb(&rio2, clientfd);

    // Response line
    linesize = Rio_readlineb(&rio2, bufB, MAXLINE);
    size += linesize; // Size of the response

    // Get response header
    while (strcmp(bufB, "\r\n") && linesize != 0)
    {
        Rio_writen(connfd, bufB, strlen(bufB));
        linesize = Rio_readlineb(&rio2, bufB, MAXLINE);
        size += linesize;
    }

    // Write to client's socket
    Rio_writen(connfd, "\r\n", 2);

    // Write response body
    while ((linesize = Rio_readnb(&rio2, bufB, MAXLINE)) != 0)
    {
        size += linesize; // Response size
        Rio_writen(connfd, bufB, linesize);
    }

    // Close client socket
    close(clientfd);

    // Run format_log_entry function (Logging)
    format_log_entry(logstring, clientaddr, uri, size);
    //
    printf("\nOpening Log file\n");
    // Open/Create proxy.log file in append mode
    file = fopen("./proxy.log", "a");
    // Put log information into the file
    fputs(logstring, file);
    fputs("\r\n", file);
    // Close the file
    fclose(file);
}

