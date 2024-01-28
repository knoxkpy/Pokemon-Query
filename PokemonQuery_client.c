#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 80


/*
This is the client part of the assignment. The main idea would be more-or-less the same with A3 but instead of performing the search in the same program, it delegates it to the server to do it. The client mainly serves for getting the search result from server, and then saving the result in a link list. When the user want to output the result, it creates the file and write the output to the file, which is same as A3. It disconnects from the server once the user chooses to exit the program.

A rough design for this server would be:
1. Open socket
2. connect to the server
3. let the user choose what's their action.
4. Client gets the result from server if the user chooses 1
5. Client saves the result if the user chooses 2
6. Client exits if the user chooses 3.
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
	int successfulInquiry;
	int totalFileNo;
	char fileCreated[40][20];
	linkedListSearchResult* outcome;
} inquiry;


void addPokemonList(linkedListSearchResult* Pokemon, inquiry* query);
void* saveResults(void* file);

inquiry query;
sem_t semQuery;

int main() {
	int clientSocket;
	struct sockaddr_in serverAddress;
	int status, bytesRcv;
	int userOption;
	char type1Search[30];
	char buffer[5000];
	char savingFileName[20];
	
	query.outcome = NULL;
	
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (clientSocket < 0) {
		printf("Unable to establish connection to the PPS!\n");
		exit(-1);
	}
	
	memset(&serverAddress, 0, sizeof(serverAddress));	if (serverSocket < 0):
		error checking
	status = bind the sockets
	if (status < 0):
		error checking

	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);
	
	status = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if (status < 0) {
		printf("CLIENT ERROR: Could not connect.\n");
		exit(-1);
	}
	
	
	printf("Successfully connected to PPS! Please choose your action:\n");
	
	query.successfulInquiry = 0;
	query.totalFileNo = 0;
	
	sem_init(&semQuery, 0, 1);
	
	while (1) {
		printf("1. Type search\n");
		printf("2. Save results\n");
		printf("3. Exit the program\n");
		
		printf("Please enter your option (Please only type-in one integer):\n");
		scanf("%d", &userOption);
		
		if (userOption == 1) {
			while (1) {
				printf("Please input the type1 of the pokemons (Or type 'Stop' to disconnect the server and exit the program/ Type 'END' to exit search): \n");
				scanf("%s", type1Search);
				
				if (strcmp(type1Search, "Stop") == 0) {
					send(clientSocket, type1Search, sizeof(type1Search), 0);
					close(clientSocket);
					printf("Disconnecting and exiting the program...");
					exit(0);
				}
				
				if (strcmp(type1Search, "END") == 0) {
					break;
				}
				
				send(clientSocket, type1Search, sizeof(type1Search), 0);
			
				memset(buffer, 0, sizeof(buffer));
				
				while (1) {
					bytesRcv = recv(clientSocket, buffer, sizeof(buffer) -1, 0);
					if (bytesRcv <= 0) {
						break;
					}
					buffer[bytesRcv] = '\0';
					if (strstr(buffer, "end") != NULL) {
					    break;
					}
					//printf("buffer: %s", buffer); (for testing only))
				}
				
				
				linkedListSearchResult* newResult = (linkedListSearchResult*) malloc(sizeof(linkedListSearchResult));
				
				sscanf(buffer, "%d,%[^,],%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", &newResult->ID, newResult->name, newResult->type1, newResult->type2, &newResult->total, &newResult->hp, &newResult->attack, &newResult->defense, &newResult->spAtk, &newResult->spDef, &newResult->speed, &newResult->generation, newResult->legendary);
				addPokemonList(newResult, &query);
			}
		} else if (userOption == 2) {
			//use case4, this part is the same as A3.
			pthread_t save;
			printf("Please enter the file name which saves the searching result: \n");
			scanf("%s", savingFileName);
			
			if (access(savingFileName, F_OK) == -1) {
				strcpy(query.fileCreated[query.totalFileNo], savingFileName);
				query.totalFileNo++;
			}
			
			pthread_create(&save, NULL, saveResults, savingFileName);
		
		
		} else if (userOption == 3) {
			printf("The total number of  queries completed successfully during the session is: %d\n", query.successfulInquiry);
			
			if (query.totalFileNo!= 0) {
				printf("The new file created during the session...\n");
				for (int i=0; i<query.totalFileNo; i++) {
					printf("%s\n", query.fileCreated[i]);
				}
			} else {
				printf("There is no file created.\n");
			}
			
			send(clientSocket, "Stop", 4, 0);
			printf("Exiting the program...\n");
						
			break;
		} else {
			//clean the input.
			while(getchar() != '\n');
			printf("Please input a correct or proper integer.\n");
		}
	}
	
	//free the memory
	linkedListSearchResult* current = query.outcome;
	while (current != NULL) {
		linkedListSearchResult* next = current -> next;
		free(current);
		current = next;
	}
	
	close(clientSocket);
	return 0;
}

void addPokemonList(linkedListSearchResult* Pokemon, inquiry* query) {
	if (query -> outcome == NULL) {
		query -> outcome = Pokemon;
	} else {
		linkedListSearchResult* current = query->outcome;
		while (current -> next != NULL) {
			current = current -> next;
		}
		current -> next = Pokemon;
	}

}

void* saveResults(void* file) {
	char* fileName = (char*)file;
	
	FILE* outputFile = fopen(fileName, "w");
	if (outputFile == NULL) {
		printf("Error writing the file.");
		pthread_exit(NULL);
	}
	
	sem_wait(&semQuery);
	linkedListSearchResult* output = query.outcome;
	
	while (output != NULL) {
		fprintf(outputFile, "%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n", output -> ID, output -> name, output -> type1, output -> type2, output -> total, output -> hp, output -> attack, output -> defense, output -> spAtk, output -> spDef, output -> speed, output -> generation, output -> legendary);
		
		output = output -> next;
	
	}
	
	sem_post(&semQuery);

	fclose(outputFile);
	
	pthread_exit(NULL);
}


