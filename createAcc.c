#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Account
{
	int acc_no;
};

int main()
{
	umask(0);
	int fd=open("Account_No.txt",O_CREAT|O_RDWR,0744);
	struct Account acc;
	acc.acc_no=0;
	write(fd,&acc,sizeof(acc));
	close(fd);
}