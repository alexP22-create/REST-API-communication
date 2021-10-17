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

//efectueaza o inregistrare si returneaza raspunsul serverului
void registerUser(int sockfd) {
	char **body_data = (char**) calloc(2, sizeof(char*));
	body_data[0] = (char*) calloc(100, sizeof(char));
	body_data[1] = (char*) calloc(100, sizeof(char));

	printf("username=");
	scanf("%s", body_data[0]);
	printf("password=");
	scanf("%s", body_data[1]);

	//crearea mesajului de trimis la server
	char* message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/register",
		"application/json", body_data, 2, NULL, 0, NULL, 0, 0);

	//afisare si trimitere
	puts(message);
	send_to_server(sockfd, message);

	//primire raspuns de la server
	char* response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);
	}

	puts(response);

	//verificare raspuns
	if (strncmp(response + 9, "201", 3) == 0) {
		printf("[SUCCES] The registration was successful.\n");
	}
	if (strncmp(response + 9, "400", 3) == 0) {
		char *msg;
		msg = strstr(response, "The");
		msg = strtok(msg, "\"");
		printf("[ERROR] %s\n", msg);
	}	

	//eliberare memorie
	free(message);
	free(body_data[0]);
	free(body_data[1]);
	free(body_data);
}

//trimite un mesaj de login si retine un cookie in lista de cookieuri
//daca nu se poate loga returneaza NULL
char* loginCommand(int sockfd) {
	char **body_data = (char**) calloc(2, sizeof(char*));
	body_data[0] = (char*) calloc(100, sizeof(char));
	body_data[1] = (char*) calloc(100, sizeof(char));

	printf("username=");
	scanf("%s", body_data[0]);
	printf("password=");
	scanf("%s", body_data[1]);

	//crearea mesajului de trimis la server
	//metoda asta se va ocupa si de cookies
	char* message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/login",
		"application/json", body_data, 2, NULL, 0, NULL, 0, 0);

	//afisare si trimitere
	puts(message);
	send_to_server(sockfd, message);

	//primire raspuns de la server
	char* response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}
	puts(response);

	//extragere si returnare cookie
	char *cookie = strstr(response, "connect");
	
	//verificam daca a avut succes sau nu
	if (cookie != NULL)
		cookie = strtok(cookie, ";");

	//eliberare memorie
	free(message);
	free(body_data[0]);
	free(body_data[1]);
	free(body_data);

	return cookie;
}

//cere acces la biblioteca
char* enter_libraryCommand (int sockfd, char** cookies, int nrCookies) {
	//creare mesaj
	char* message = compute_get_request("34.118.48.238", "/api/v1/tema/library/access",NULL , cookies, nrCookies, NULL, 0);
	send_to_server(sockfd, message);
	char* response = receive_from_server(sockfd);
	
	if  (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}	

	puts(response);

	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] The acces to library was successful.\n");
	} else {
		printf("[ERROR] You have to log in first\n");
		return NULL;
	}

	char *token = (strstr(response, "{")) + 10;
	token = strtok(token, "\"");

	return token;
}

//adauga o carte in librarie
void addOneBook(int sockfd, char** tokens, int nrTokens) {
	char title[50], author[50], genre[20], publisher[20];
	char page_count[5];  
	printf("title=");
	scanf("%s",title);
	printf("author=");
	scanf("%s",author);
	printf("genre=");
	scanf("%s",genre);
	printf("publisher=");
	scanf("%s",publisher);
	printf("page_count=");
	scanf("%s",page_count);			

	//verificare ca page_count-ul este valid
	for(int i = 0; i < strlen(page_count); i++) {
		if (page_count[i] < 48 || page_count[i] > 57) {
			printf("Page_count invalid.\n");
			return;
		}
	}

	//alocare date
	char **body_data = (char**)calloc(5, sizeof(char*));
	for(int i = 0; i < 5; i++) {
		body_data[i] = (char*)calloc(100,sizeof(char));
	}

	memcpy(body_data[0], title, 50 * sizeof(char));
	memcpy(body_data[1], author, 50 * sizeof(char));
	memcpy(body_data[2], genre, 20 * sizeof(char));
	memcpy(body_data[3], publisher, 20 * sizeof(char));
	memcpy(body_data[4], page_count, 5 * sizeof(char));

	//trimitere mesaj
	char *message = compute_post_request("34.118.48.238", "/api/v1/tema/library/books",
		"application/json", body_data, 5, NULL, 0, tokens, nrTokens, 1);
	puts(message);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}
	puts(response);

	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] The book was added to the library.\n");
	} else {
		printf("[ERROR] Couldn't add the book to the library.\n");
	}

	free(message);
	free(response);
    for(int i = 0; i < 5; i++)
        free(body_data[i]);
    free(body_data);	

}

//afiseaza cartile de pe server
void printBooks(int sockfd, char** tokens, int nrTokens) {
	//creare mesaj
	char* message = compute_get_request("34.118.48.238", "/api/v1/tema/library/books", NULL, NULL, 0, tokens, nrTokens);
	send_to_server(sockfd, message);
	char* response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}	

	puts(response);

	char *books = strstr(response, "{");
	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] This are all the books:\n");
		printf("%s\n", books);
	} else {
		printf("[ERROR] Can't show the books.\n");
	}

	free(message);
	free(response);
}

// afiseaza o anumita carte dupa id
void printOneBook(int sockfd, char** tokens, int nrTokens) {
	char *id = malloc(10 * sizeof(char));
	printf("id=");
	scanf("%s", id);

//verificare ca id-ul este valid
	int okId = 1;
	for(int i = 0; i < strlen(id); i++) {
		if (id[i] < 48 || id[i] > 57) {
			okId = 0;
		}
	}
	if (okId == 0) {
		printf("[Error]Id invalid.\n");
		return;
	}
	char url[] = "/api/v1/tema/library/books/";
	strcat(url, id);
	char *message = compute_get_request("34.118.48.238", url, NULL, NULL, 0, tokens, nrTokens);
	puts(message);
	send_to_server(sockfd, message);
	
	char *response = receive_from_server(sockfd);
	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}	

	puts(response);
	char *book = strstr(response, "{");
	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] This is the wanted book:\n");
		printf("%s\n", book);
	} else {
		printf("[ERROR] There are no books with the given ID.\n");
	}	

	free(message);
	free(response);
}

//elimina o carte din biblioteca
void deleteOneBook(int sockfd, char **tokens, int nrTokens) {
	char *id = malloc(10 * sizeof(char));
	printf("id=");
	scanf("%s",id);

	//verificare ca id-ul este valid
	int okId = 1;
	for(int i = 0; i < strlen(id); i++) {
		if (id[i] < 48 || id[i] > 57) {
			okId = 0;
		}
	}
	if (okId == 0) {
		printf("[ERROR]Id invalid.\n");
		return;
	}
	char url[] = "/api/v1/tema/library/books/";
	strcat(url, id);
	char *message = compute_delete_request("34.118.48.238", url, NULL, tokens, nrTokens);
	puts(message);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}	

	puts(response);

	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] The book was deleted from the library.\n");
	} else {
		printf("[ERROR] Couldn't delete the book from the library.\n");
	}	

	//eliberare memorie
	free(message);
	free(response);	
}

//delogheaza un client
void logoutCommand(int sockfd, char **cookies, int nrCookies) {
	char *message = compute_get_request("34.118.48.238", "/api/v1/tema/auth/logout", NULL,
		cookies, nrCookies, NULL, 0);
	puts(message);
	send_to_server(sockfd, message);
	char *response = receive_from_server(sockfd);

	if (strlen(response) == 0) {
		//refacere conexiune si retrimitere mesaj
		sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);		
	}	

	puts(response);

	//verificare raspuns
	if (strncmp(response + 9, "200", 3) == 0) {
		printf("[SUCCES] You logged out.\n");
	} else {
		printf("[ERROR] Couldn't log out.\n");
	}

	free(message);
	free(response);	

}