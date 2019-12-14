#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"

#define MAX_THREADS 100

int exitSafe;

struct client
{
	struct Calc *calc;
	int infd, outfd;
	int id;
};

/* buffer size for reading lines of input from user */
#define LINEBUF_SIZE 1024

void chat_with_client(struct Calc *calc, int infd, int outfd);
void *connection_handler(void *cli);
int input_timeout (int filedes, unsigned int seconds);

int main(int argc, char const *argv[])
{ 
	void* status;
    pthread_t threads[MAX_THREADS];
	exitSafe = 0;
	int id = 0;
    int serverSocket, clientSocket; 
    char port[16];
    struct Calc *calc = calc_create();
    if (argc != 2)
    {
    	printf("Wrong Sintaxis.\nPlease use: ./calcServer <port>\n");
    	calc_destroy(calc);
    	return 1;
    }
    strcpy(port, argv[1]);
	serverSocket = open_listenfd(port);

	/* Iterate until a thread ask 'shutdown' */
	while(exitSafe == 0) {
		/* Check if the socket is active */
		if (input_timeout(serverSocket, 1) == 1)
		{
			clientSocket = Accept(serverSocket, NULL, NULL); // Accept a new conection
		    if (clientSocket < 0) { 
		        printf("server acccept failed...\n"); 
		        return 1; 
		    } 
		    else
		        printf("server acccept the client...\n"); 

	        struct client *cl = (struct client*) malloc(sizeof(struct client));
	        cl->calc = calc;
	        cl->id = id;
	        cl->infd = clientSocket;
	        cl->outfd = clientSocket;

	        if( pthread_create(&threads[id], NULL,  connection_handler, (void*) cl) < 0)
	        {
	            perror("could not create thread");
	            return 1;
	        }
	        id++;
		}
	}

	/* When a shutdown is received kill all threads still alive */
	for(int j = 0; j < id; j++)
	{
		pthread_cancel(threads[j]);
		pthread_join(threads[j], &status);
	}

	/* Dealloc the memory reserved for the calc structure */
	calc_destroy(calc);
  
    // After chatting close the socket 
    close(serverSocket); 
} 

void *connection_handler(void *cli) {
	struct client *cl = (struct client*) cli;	
	chat_with_client(cl->calc, cl->infd, cl->outfd);
	free(cl);
	pthread_exit(NULL);
	return NULL;
}

void chat_with_client(struct Calc *calc, int infd, int outfd) {
	rio_t in;
	char linebuf[LINEBUF_SIZE] = {0};
	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, infd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 */
	int done = 0;
	while (!done) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			/* error or end of input */
			done = 1;
		} 
		else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			Close(outfd);
			done = 1;
		} 
		else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
			Close(outfd);
			done = 1;
			exitSafe = 1; // This break the main loop waiting for the new conections
		} 
		else {
			/* process input line */
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {

				/* expression couldn't be evaluated */
				rio_writen(outfd, "Error\n", 6);
			} else {					

				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(outfd, linebuf, len);
				}
			}
		}
	}
}

/* Funtion to check if the file descripor passed as parameter is active every amount of 'seconds' */
/* Return 1 is the file descriptor is active or 0 in other case */
int input_timeout(int filedes, unsigned int seconds) {        
    fd_set set;                                             
    struct timeval timeout;
    FD_ZERO (&set);        
    FD_SET (filedes, &set);
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return (select (FD_SETSIZE, &set, NULL, NULL, &timeout));
}
