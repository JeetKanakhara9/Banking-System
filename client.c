#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<limits.h>
#define PORT 4444
struct message
{
	char msg[200];
	int  flag;
};
int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	//char buffer[100];
	struct message message;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		
		if(read(clientSocket,&message,sizeof(message)) <0){
			printf("[-]Error in receiving data.\n");
		}
		else{

			if(message.flag==0)
			{
				printf("%s",message.msg);
			}
			
			
			if(message.flag==1)
			{
				printf("%s",message.msg);
				scanf("%s",message.msg);
				write(clientSocket,&message,sizeof(message));
			}
			if(message.flag==2)
			{
				char *c=getpass(message.msg);
				strcpy(message.msg,c);
				write(clientSocket,&message,sizeof(message));
				//printf("%s\n",message.msg);
			}
		}
		

	}

	return 0;
}
