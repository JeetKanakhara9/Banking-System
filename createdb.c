#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
struct Customer
{
	double balance;
	char username1[100];
	char username2[100];
	char password1[100];
	char password2[100];
	int acc_no;
	int type;
	int active;
};

int main()
{
	umask(0);
	int fd=open("database.txt",O_CREAT|O_RDWR|O_APPEND,0744);
	if(fd==-1)
		perror("");
	else
	{
		struct Customer u1;
		strcpy(u1.username1,"admin");
		strcpy(u1.password1,"admin");
		u1.type=0;
		u1.active=1;
		u1.acc_no=0;
		u1.balance=0.0;
		lseek(fd,0,SEEK_END);
		write(fd,&u1,sizeof(u1));
		close(fd);
	}
}


		
