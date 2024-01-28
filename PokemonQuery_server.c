#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>

#define SERVER_PORT 80

/*
The main idea of this server is to accept the prompt from client and perform the type1 search and then send back the result to the user/client.

How it deal with the type search is exactly the same as what've been done in A3 but the trickiest part is to send back the response to the client

A rough design for this server would be:
1. Open socket
2. check if pokemon.csv exits.
3. connection to the client
4. recieved client prompt
5. Perform searching and storing it in a linklist
6. transforming the link list into string and stored in response
7. send the response to client.

*The printf function which is commented is used to test the code & debugging.
*/

typedef struct linkedListSearchResult {
	int ID;
	char name[40];
	char type1[15];
	char type2[15];
	int total;
	int hp;
	int attack;
	int defense;
	int spAtk;
	int spDef;
	int speed;
	int generation;
	char legendary[7];
	struct linkedListSearchResult* next;
} linkedListSearchResult;

typedef struct inquiry {
	char type1Input[15];
	char fileName[20];
	pthread_t thread;
	linkedListSearchResult* outcome;
} inquiry;

sem_t semQuery;

void* enquiry(void* arg);

int main() {

	//use case 1, but this part is almost-the-same as what've been done in A3, except we set up the socket.
	FILE *inputFile;	
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddress;
	int status, addrSize, bytesRcv;
	char buffer[50];
	char type1Search[30];
	char response[5000];
	inquiry query;
	
	
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		printf("SERVER ERROR: Could not open socket.\n");
		exit(-1);
	}
	
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);
	
	status = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (status < 0) {
		printf("SERVER ERROR: Cannot bind socket.\n");
		exit(-1);
	}
	
	
	status = listen(serverSocket, 10);
	if (status < 0) {
		printf("SERVER ERROR: Could not listen on socket.\n");
		exit(-1);
	}
	
	
	while (1) {
		printf("This is the Pokemon query program...\n");
		printf("Please enter the name of the file containing the Pokemon description, or type 'exit' to quit the program: ");
		scanf("%s", query.fileName);

		if (strcmp(query.fileName, "exit") == 0) {
			printf("exiting the program...");
			exit(0);
		}		
		
		inputFile = fopen(query.fileName, "r");
		if (inputFile == NULL) {
			printf("Pokemon file is not found. Please enter the name of the file again.\n");
		} else {
			break;
		}
	}
	
	fclose(inputFile);
	query.outcome = NULL;
	sem_init(&semQuery, 0, 1);
	
	while (1) {
		addrSize = sizeof(clientAddress);
		clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddress, &addrSize);
		if (clientSocket < 0) {
			printf("SERVER ERROR: Couldn't accept incoming client connection.\n");
			close(serverSocket);
			exit(-1);
		}
		printf("SERVER: Received client connection.\n");
		
		bytesRcv = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
		buffer[bytesRcv] = '\0';
		strcpy(type1Search, buffer);
		
		if (strcmp(buffer, "Stop") == 0) {
			printf("SERVER: Closing client connection.\n");
			close(serverSocket);
			break;
		}
		
		printf("Currently looking for Pokemon type: %s\n", type1Search);
		
		strcpy(query.type1Input, type1Search);
		
		pthread_create(&query.thread, NULL, enquiry, &query);
		pthread_join(query.thread, NULL);
		
		linkedListSearchResult* current = query.outcome;
		
		//printf("Value of query.outcome: %p\n", (void *) query.outcome); (for testing/debugging)
		while (current != NULL) {
			sprintf(response, "%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n", current -> ID, current -> name, current -> type1, 		 current -> type2, current -> total, current -> hp, current -> attack, current -> defense, current -> spAtk, current -> spDef, current -> speed, current -> generation, current -> legendary);
			//printf("%s\n", response); (for testing/debugging)
			send(clientSocket, response, strlen(response), 0);
			current = current -> next;
		}
		
		send(clientSocket, "end\n", 4, 0);
	}
	
	close(serverSocket);
	printf("SERVER: Shutting down.\n");

	
	return 0;
}

//exactly the same function from A3
void* enquiry(void* arg) {
	FILE* file;
	inquiry* searchPointer = (inquiry*)arg;
	file = fopen(searchPointer -> fileName, "r");
	if (file == NULL) {
	printf("Files cannot be opened...\n");
	pthread_exit(NULL);
	}
    
	char line[500];
	fgets(line, sizeof(line), file);

	while (fgets(line, sizeof(line), file)) {
		int ID;
		char name[40];
		char type1[15];
		char type2[15];
		int total;
		int hp;
		int attack;
		int defense;
		int spAtk;
		int spDef;
		int speed;
		int generation;
		char legendary[7];

		char *token = strtok(line, ",");
		ID = atoi(token);

		token = strtok(NULL, ",");
		strncpy(name, token, sizeof(name) -1);

		token = strtok(NULL, ",");
		strncpy(type1, token, sizeof(type1) -1);

		token = strtok(NULL, ",");
		strncpy(type2, token, sizeof(type2) -1);

		token = strtok(NULL, ",");
		total = atoi(token);

		token = strtok(NULL, ",");
		hp = atoi(token);

		token = strtok(NULL, ",");
		attack = atoi(token);

		token = strtok(NULL, ",");
		defense = atoi(token);
			
		token = strtok(NULL, ",");
		spAtk = atoi(token);

		token = strtok(NULL, ",");
		spDef = atoi(token);

		token = strtok(NULL, ",");
		speed = atoi(token);
			
		token = strtok(NULL, ",");
		generation = atoi(token);
		
		token = strtok(NULL, "\n");
		if (token != NULL) {
			strncpy(legendary, token, sizeof(legendary) - 1);
		}
		

		if (strcmp(type1, searchPointer -> type1Input) == 0) {
			linkedListSearchResult* searchResult = malloc(sizeof(linkedListSearchResult));
		    
			searchResult -> ID = ID;
			strcpy(searchResult -> name, name);
			strcpy(searchResult -> type1, type1);
			
			//if no types 2, copy nothing but a empty string.
			if (type2[0] != '\0') {
				strcpy(searchResult -> type2, type2);
			} else {
				strcpy(searchResult -> type2, " ");
			}
		    
			searchResult -> total = total;
			searchResult -> hp = hp;
			searchResult -> attack = attack;
			searchResult -> defense = defense;
			searchResult -> spAtk = spAtk;
			searchResult -> spDef = spDef;
			searchResult -> speed = speed;
			searchResult -> generation = generation;
			strcpy(searchResult -> legendary, legendary);
				    
			sem_wait(&semQuery);
			searchResult -> next = searchPointer -> outcome;
			searchPointer -> outcome = searchResult;
			sem_post(&semQuery);
			}
	}
    
	fclose(file);
    
	pthread_exit(NULL);
}

