/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sendfile.h>
#define BUFF_SIZE 255

void error(char *msg)
{
    perror(msg);
    exit(1);
}


char * create_header(int fsize, char * contenttype)
{
    char * header = (char*)malloc(256);
    bzero(header, 256);

    char * basic ="HTTP/1.1 200 OK\r\n"
                "Server: Apache\r\n";
    
    int rsize = 0;
    rsize += sprintf(header,"%s", basic);
    rsize += sprintf(header+rsize, "Content-Length: %d\r\n", fsize);
    rsize += sprintf(header+rsize, "Content-Type: %s\r\n", contenttype);
    rsize += sprintf(header+rsize, "\r\n");

    return header;
}
char * set_content_type(char * contenttype, char * requestfile)
{
    printf("%s\n", requestfile);

    char * extension = strstr(requestfile, ".") + 1;
    printf("%s\n", extension);
    if (strcmp(extension, "html") == 0)
    {
        sprintf(contenttype, "%s", "text/html");
    }
    else if(strcmp(extension, "gif") == 0)
    {
        sprintf(contenttype, "%s", "image/gif");
    }
    else if(strcmp(extension, "jpeg") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0)
    {
        sprintf(contenttype, "%s", "image/jpeg");
    }
    else if(strcmp(extension, "mp3") == 0)
    {
        sprintf(contenttype, "%s", "audio/mpeg3"); //firefox
    }
    else if(strcmp(extension, "pdf") == 0)
    {
        sprintf(contenttype, "%s", "application/pdf");
    }            
    else
    { 
        printf("not support such extension\n");
        sprintf(contenttype, "%s", "404");  
    }

    printf("%s\n", contenttype);

    return contenttype;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd; //descriptors rturn from socket and accept system calls
    int portno; // port number
    /* socklen_t : used for socket-related parameters. definition of length and size values */
    socklen_t clilen; 
    
    char buffer[256];
    char fcontent[2048];
    char copyrt[10];
    char * contenttype = (char*)malloc(30);

    /*sockaddr_in: Structure Containing an Internet Address*/
    /* 
    sin_family : address system(AF_INET) 2 bytes
    sin_port : 16 bits port num (0~2^16-1)
    sin_addr : 32 bits IP address
    sin_zero[8] : dummy to fit the entire size to 16 bits (8+8 = 16 bytes / sockaddr => 16 bytes)
    */
    struct sockaddr_in serv_addr, cli_addr;

    int n, fsize;

    FILE * fp;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bzero((char *) &cli_addr, sizeof(cli_addr));


    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
     
    /*Create a new socket
       AF_INET: Address Domain is Internet 
       SOCK_STREAM: Socket Type is STREAM Socket(TCP) 
       IPPROTO_HOPOPTS = 0
    */
    /* socket(af, type, protocol)
    af : communication over the internet / processes's communication within the same system
    type : socket type(TCP / UDP)
    protocol : variables for specifying the use of protocol in communications
    return : identifier of socket / failure(-1)
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    

    portno = atoi(argv[1]); //atoi converts from String to Integer
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
    serv_addr.sin_port = htons(portno); //convert from host to network byte order
    /* bind(int sockfd, sockaddr *myaddr, socklen_t addrlen)
    sockfd : socket descriptor(socket I made)
    myaddr : server address
    addrlen : size of myaddr struct
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
        error("ERROR on binding");
    /*int listen(int s, int backlog)   
    s : socket descriptor
    backlog : # of waiting message queue
    return : 0(success) / -1(failure)
    */
    listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5
    

    clilen = sizeof(cli_addr);
    do{
        // initialize (memset(temp, '/0', sizeof(temp)))
        bzero(buffer,sizeof(buffer));
        bzero(fcontent, sizeof(fcontent));
        bzero(copyrt,sizeof(copyrt));
        bzero(contenttype, 30);

        /*accept function: 
           1) Block until a new connection is established
           2) the new socket descriptor will be used for subsequent communication with the newly connected client.
        */
        /*int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
        s : socket descriptor
        addr: client address
        addrlen : size of addr struct
        return : identifier of new socket / failure(-1)
        */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");


        /*read(client_socket, buff_rcv, BUFF_SIZE)*/
        n = read(newsockfd,buffer,BUFF_SIZE); //Read is a block function. It will read at most 255 bytes
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        char * requesttype = strtok(buffer, " ");
        char * file = strtok(NULL, " ");
        char requestfile[strlen(file)];

        strcpy(requestfile, file+1);

        strcpy(copyrt, requesttype);
        printf("%s\n", requesttype);

        if (strcmp(requesttype,"GET") == 0)
        {
            if (strlen(requestfile) == 0)
            {
                sprintf(contenttype, "text/html");
                fp = fopen("home.html","rb");
            }
            else{
                fp = fopen(requestfile,"rb");
                if (strcmp(contenttype, "404") == 0 || fp ==NULL)
                {
                    sprintf(contenttype, "text/html");
                    fp = fopen("404.html","rb");
                }
                else
                    sprintf(contenttype, "%s", set_content_type(contenttype, requestfile));

            }
            /*int fseek( FILE *stream, long offset, int origin )
            stream : pointer of open file structure
            offset: offset from origin
            origin : SEEK_SET(move from BOF to offset)/ SEEK_CUR(move from current location of file pointer to offset)/ SEEK_END(move from EOF to offset)
            return : success(0) / failure(another value)
            
            ftell(file) : current location of file
            */

            fseek(fp, 0L, SEEK_END);
            fsize = ftell(fp);
            printf("%d\n", fsize);
            rewind(fp);
            
            char * header = create_header(fsize, contenttype);
            /*write(client_socket, buff_snd, strlen(buff_snd))*/
            n = write(newsockfd,header,strlen(header)); //NOTE: write function returns the number of bytes actually sent out Ñ> this might be less than the number you told it to send
            int i = 0;
            if (n < 0) error("ERROR writing to socket");
            while((n = fread(fcontent, 1, 2048, fp)) != 0){
                n = write(newsockfd,fcontent,n);
                bzero(fcontent,sizeof(fcontent));
                if (n < 0) error("ERROR writing to socket");
                i += n;
            }
            printf("%d\n", i);

            free(header);
            printf("%p\n", contenttype);
            close(newsockfd);
        }

    }while(1);
    free(contenttype);
    close(sockfd);

    return 0; 
}
