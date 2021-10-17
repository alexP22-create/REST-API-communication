// Postolache Alexanru-Gabriel
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "commands.c"
//function which tells us what to do based on command
int applyFunction(char* command) {
	if (strncmp(command, "register", 8) == 0) {
		return 0;
	}
	if (strncmp(command, "login", 5) == 0) {
		return 1;
	}
	if (strncmp(command, "enter_library", 13) == 0) {
		return 2;
	}
	if (strncmp(command, "get_books", 9) == 0) {
		return 3;
	}
	if (strncmp(command, "get_book", 8) == 0) {
		return 4;
	}
	if (strncmp(command, "add_book", 8) == 0) {
		return 5;
	}
	if (strncmp(command, "delete_book", 11) == 0) {
		return 6;
	}
	if (strncmp(command, "logout", 6) == 0) {
		return 7;
	}
	if (strncmp(command, "exit", 4) == 0) {
		return 8;
	}
	return -1;
}

int main(int argc, char *argv[])
{
    int sockfd;

    char command[20];

	//lista de cookies // tablou deoarece asta cer fct din requests.c
	char **cookies = (char**)calloc(10, sizeof(char*));
	for (int i = 0; i < 5; i++) {
		cookies[i] = (char*)calloc(400, sizeof(char));
	}

	//tokenuri
	char **tokens = (char**)calloc(10, sizeof(char*));
	for (int i = 0; i < 5; i++) {
		tokens[i] = (char*)calloc(400, sizeof(char));
	}	

	//suntem logati daca >=1
	int nrCookies = 0;

	//avem acces la biblioteca daca >=1
	int nrTokens = 0;

	while(1) {
		//deschidere conexiune socket
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

		fgets(command, 19, stdin);
		int whichFunc = applyFunction(command);

		if (whichFunc == 0) {
			registerUser(sockfd);
		}
		if (whichFunc == 1) {
			if (nrCookies == 0) {
				char* cookie = loginCommand(sockfd);

				if (cookie == NULL) {
					printf("[ERROR] The user didn't logged in\n");
				} else {
					nrCookies = 1;
					//adaugare in tabel
					memcpy(cookies[0],cookie, 400*sizeof(char));
					printf("[SUCCES] The user logged in with cookie: %s.\n", cookies[0]);
				}
			}
			else {
				printf("[ERROR] A user is already connected.\n");
			}
		}
		if (whichFunc == 2) {
			if(nrCookies == 1) {
				char* token = enter_libraryCommand(sockfd, cookies, nrCookies);
				if (token != NULL) {
					nrTokens = 1;
					//adaugare in tabel
					memcpy(tokens[0], token, 400 * sizeof(char));
				}
			}
			else {
				printf("[ERROR] You have to log in first.\n");
				continue;
			}
		}
		if (whichFunc == 3) {
			if (nrTokens == 1) {
				printBooks(sockfd, tokens, nrTokens);
			}
			else {
				printf("[ERROR] You don't have acces to the library.\n");
			}		
		}
		if (whichFunc == 4) {
			if (nrTokens == 1) {
				printOneBook(sockfd, tokens, nrTokens);
			}
			else {
				printf("[ERROR] You don't have acces to the library.\n");
			}
		}
		if (whichFunc == 5) {
			if (nrTokens == 1) {
				addOneBook(sockfd, tokens, nrTokens);
			}
			else {
				printf("[ERROR] You don't have acces to the library.\n");
			}			 
		}
		if (whichFunc == 6) {
			if (nrTokens == 1) {
				deleteOneBook(sockfd, tokens, nrTokens);
			}
			else {
				printf("[ERROR] You don't have acces to the library.\n");
			}			
		}
		if (whichFunc == 7) {
			if (nrCookies == 1) {
				logoutCommand(sockfd, cookies, nrCookies);
				nrCookies = 0;
				nrTokens = 0;
			}
			else {
				printf("[ERROR] You have to log in first.\n");
			}
		}
		if (whichFunc == 8) {
			break;
		}
	}

	close(sockfd);
    return 0;
}
