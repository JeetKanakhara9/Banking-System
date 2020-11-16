#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<time.h>
#define PORT 4444

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

struct message
{
	char msg[200];
	int  flag;
};
struct Account
{
	int acc_no;
};

struct log
{
	char date[50];
	char time[50];
	char c;
	double amount;
	double balance;
};
void msgtoclient(int client,char str[],int flag)
{
	struct message message;
	strcpy(message.msg,str);
	message.flag=flag;
	write(client,&message,sizeof(message));
}

void msgfromclient(int client,char str[])
{	
	struct message message;
	read(client,&message,sizeof(message));
	strcpy(str,message.msg);	
}
struct Customer getuser(char name[])
{
	struct Customer user;
	struct flock lock;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDONLY);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	int flag=1;
	while(read(fd,&user,sizeof(user))>0)
	{
		if(strcmp(name,user.username1)==0&&user.active==1)
		{
			flag=-1;
			break;
		}
		if(strcmp(name,user.username2)==0&&user.active==1)
		{
			flag=-1;
			break;
		}
	}
	//flag=1;
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	return user;	
}
struct Customer auth(char name[],char pass[])
{
	struct Customer user;
	struct Customer user1;
	user1.active=0;	
	struct flock lock;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDONLY);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	
	
	while(read(fd,&user,sizeof(user))>0)
	{
		if(strcmp(name,user.username1)==0&&strcmp(pass,user.password1)==0&&user.active==1)
		{
			lock.l_type=F_UNLCK;
			fcntl(fd,F_SETLK,&lock);
			return user;
		}
		if(strcmp(name,user.username2)==0&&strcmp(pass,user.password2)==0&&user.active==1)
		{	lock.l_type=F_UNLCK;
			fcntl(fd,F_SETLK,&lock);
			return user;
		}
	}
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	return user1;
}
void getInfo(int client,struct Customer user)
{

		struct Customer user1=getuser(user.username1);
		char n[]="\nUsername : ";
		msgtoclient(client,n,0);
		msgtoclient(client,user.username1,0);
		if(user.type==2)
		{
			char m[]="\nUsername 2 : ";
			msgtoclient(client,m,0);
			msgtoclient(client,user.username2,0);	
		}
		char acc[]="\nAccount Number : ";
		msgtoclient(client,acc,0);
		char acc_no[20];
		sprintf(acc_no,"%d",user.acc_no);
		msgtoclient(client,acc_no,0);
		char b[]="\nBalance : ";
		msgtoclient(client,b,0);
		sprintf(b,"%f",user1.balance);
		msgtoclient(client,b,0);

}

void getBalance(int client,struct Customer user)
{
		struct Customer user1=getuser(user.username1);
		char b[]="\nBalance : ";
		msgtoclient(client,b,0);
		char balance[100];
		sprintf(balance,"%f",user1.balance);
		msgtoclient(client,balance,0);
			
}
void update_log(int acc_no,char c,double amount,double balance)
{
	time_t t=time(NULL);
	struct tm tm=*localtime(&t);
	char d[50],ti[50];
	sprintf(d,"%02d/%02d/%04d",tm.tm_mday,tm.tm_mon+1,tm.tm_year+1900);
	sprintf(ti,"%02d:%02d:%02d",tm.tm_hour,tm.tm_min,tm.tm_sec);
	struct log log;
	strcpy(log.date,d);
	strcpy(log.time,ti);
	log.c=c;
	log.amount=amount;
	log.balance=balance;
	char acc[30];
	sprintf(acc,"%d",acc_no);
	strcat(acc,".txt");
	struct flock lock;
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	umask(0);
	int fd=open(acc,O_CREAT|O_RDWR|O_APPEND,0744);
	if(fd==-1)
		perror("");
	fcntl(fd,F_SETLKW,&lock);
	lseek(fd,0,SEEK_END);
	write(fd,&log,sizeof(log));
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);	

}

void print_log(int client,struct Customer user)
{
	struct log log;
	char acc[30],amnt[20],bal[20];
	sprintf(acc,"%d",user.acc_no);
	strcat(acc,".txt");
	struct flock lock;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open(acc,O_RDONLY);
	fcntl(fd,F_SETLKW,&lock);
	lseek(fd,0,SEEK_SET);
	//printf("Inside printlog\n");
	while(read(fd,&log,sizeof(log))>0)
	{
		//printf("Inside read\n");
		msgtoclient(client,log.date,0);
		msgtoclient(client," ",0);
		msgtoclient(client,log.time,0);
		msgtoclient(client," ",0);
		msgtoclient(client,&log.c,0);
		msgtoclient(client," ",0);
		sprintf(amnt,"%f",log.amount);
		msgtoclient(client,amnt,0);
		msgtoclient(client," ",0);
		sprintf(bal,"%f",log.balance);
		msgtoclient(client,"Current Available Balance : ",0);
		msgtoclient(client,bal,0);
		msgtoclient(client,"\n",0);
	}
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
}
void Credit(int client,struct Customer user)
{
	/*Now doing record locking according to account number as it is unique */	struct Customer user1;
	struct flock lock;
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=(user.acc_no)*sizeof(user);
	lock.l_len=sizeof(user);
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDWR);
	fcntl(fd,F_SETLKW,&lock);
	lseek(fd,(user.acc_no)*sizeof(user),SEEK_SET);
	read(fd,&user1,sizeof(user1));
	double amount;
	char b[]="\nCurrent Balance : ";
	msgtoclient(client,b,0);
	char balance[20];
	sprintf(balance,"%f",user1.balance);
	msgtoclient(client,balance,0);
	char b1[]="\nEnter amount to be credited : ";
	msgtoclient(client,b1,1);
	char amnt[20];
	char *end;
	msgfromclient(client,amnt);
	amount=strtod(amnt,&end);
	user1.balance+=amount;
	lseek(fd,-1*sizeof(user),SEEK_CUR);
	write(fd,&user1.balance,sizeof(user1.balance));
	update_log(user.acc_no,'C',amount,user1.balance);
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	msgtoclient(client,"\nAmount Credited Successfully\n",0);
	
}

void Debit(int client,struct Customer user)
{
	/*Now doing record locking according to account number as it is unique */	struct Customer user1;
	struct flock lock;
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=(user.acc_no)*sizeof(user);
	lock.l_len=sizeof(user);
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDWR);
	fcntl(fd,F_SETLKW,&lock);
	lseek(fd,(user.acc_no)*sizeof(user),SEEK_SET);
	read(fd,&user1,sizeof(user1));
	double amount;
	char b[]="\nCurrent Balance : ";
	msgtoclient(client,b,0);
	char balance[20];
	sprintf(balance,"%f",user1.balance);
	msgtoclient(client,balance,0);
	char b1[]="\nEnter amount to be debited : ";
	msgtoclient(client,b1,1);
	char amnt[20];
	char *end;
	msgfromclient(client,amnt);
	amount=strtod(amnt,&end);
	//printf("%f\n",amount)
	if(amount>user1.balance)
          {
		char b2[]="\nInsufficient Balance Available\n";
		msgtoclient(client,b2,0);
	}
	else
	{
	user1.balance-=amount;
	lseek(fd,-1*sizeof(user),SEEK_CUR);
	write(fd,&user1.balance,sizeof(user1.balance));
	update_log(user.acc_no,'D',amount,user1.balance);
	msgtoclient(client,"\nAmount Debited Successfully\n",0);
	}
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	
	
}

struct Customer auth_u(char name[])
{
	struct Customer user;
	struct Customer user1;
	user1.active=0;	
	struct flock lock;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDONLY);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	
	
	while(read(fd,&user,sizeof(user))>0)
	{
		if(strcmp(name,user.username1)==0&&user.active==1)
		{
			lock.l_type=F_UNLCK;
			fcntl(fd,F_SETLK,&lock);
			return user;
		}
		if(strcmp(name,user.username2)==0&&user.active==1)
		{
			lock.l_type=F_UNLCK;
			fcntl(fd,F_SETLK,&lock);
			return user;
		}
	}
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	return user1;	
}
void getInfo_Admin(int client)
{
		char u[]="\nEnter Username : ";
		msgtoclient(client,u,1);
		char name[100];
		msgfromclient(client,name);
		struct Customer user;
		user=auth_u(name);
		char s[]="Valid User\n";
		char s1[]="Invalid Username\nEnter Username: ";
		while(user.active==0)
		{
			msgtoclient(client,s1,1);
			msgfromclient(client,name);
			user=auth_u(name);
		}
		msgtoclient(client,s,0);
		getInfo(client,user);

}
int checkUser(char name[])
{
	struct Customer user;
	struct flock lock;
	lock.l_type=F_RDLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDONLY);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	int flag=1;
	while(read(fd,&user,sizeof(user))>0)
	{
		if(strcmp(name,user.username1)==0&&user.active==1)
		{
			flag=-1;
			break;
		}
		if(strcmp(name,user.username2)==0&&user.active==1)
		{
			flag=-1;
			break;
		}
	}
	//flag=1;
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	return flag;	
}
int getnewAcc_no()
{
	struct Account a;
	struct flock lock;
	int fd=open("Account_No.txt",O_RDWR);
	if(fd==-1)
	{
		perror("");
	}
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLKW,&lock);
	read(fd,&a,sizeof(a));
	a.acc_no++;
	lseek(fd,0,SEEK_SET);
	write(fd,&a,sizeof(a));
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	return a.acc_no;
}
void create_new(struct Customer user)
{
	struct flock lock;
	int fd=open("database.txt",O_RDWR);
	if(fd==-1)
	{
		perror("");
	}
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	fcntl(fd,F_SETLKW,&lock);
	lseek(fd,0,SEEK_END);
	write(fd,&user,sizeof(user));
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
}
int join(int client,char name1[],char name[],char pass[])
{
	struct Customer user;
	struct flock lock;
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDWR);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	int flag=-1;
	while(read(fd,&user,sizeof(user))>0)
	{
		if((strcmp(name1,user.username1)==0||strcmp(name1,user.username2)==0)&&user.active==1)
			{
				if(user.type==2)
				{
					char m[]="Already a Joint Account User..Can't Join\n";
					msgtoclient(client,m,0);
					flag=0;
					break;
				}
				if(user.type==1)
				{
					user.type=2;
					strcpy(user.username2,name);
					strcpy(user.password2,pass);
					lseek(fd,(user.acc_no)*sizeof(user),SEEK_SET);
					write(fd,&user,sizeof(user));
					flag=1;
					break;
				}

			}
	}
	//flag=1;
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	close(fd);
	return flag;	
}
void Join_Existing(int client,char name[],char pass[])
{
	char m[]="\nEnter username which you want to join : ";
	msgtoclient(client,m,1);
	char name1[100];
	msgfromclient(client,name1);
	int r=join(client,name1,name,pass);
	char s[]="\nUsername does not exist ..Enter Different Username : ";
	while(r!=1)
	{
			if(r==0)
			{
				msgtoclient(client,"\nEnter Different Username : ",1);
			}
			else
			{
				msgtoclient(client,s,1);
			}
			msgfromclient(client,name1);
			r=join(client,name1,name,pass);
	}
	msgtoclient(client,"\nJoint Account Created Successfully\n",0);
}
void Joint_New(int client,char name[],char pass[])
{
	msgtoclient(client,"\nEnter second username : ",1);
	char name1[100];
	msgfromclient(client,name1);
	int r=checkUser(name1);
	char s[]="Username Already Exists..Please Enter Different Username : ";
	while(r==-1)
	{
		msgtoclient(client,s,1);
		//msgtoclient(client,u,1);
		msgfromclient(client,name1);
		r=checkUser(name1);		
	}
	char pass1[100];
	msgtoclient(client,"\nEnter password for user2 : ",2);
	msgfromclient(client,pass1);
	struct Customer user;
	strcpy(user.username1,name);
	strcpy(user.username2,name1);
	strcpy(user.password1,pass);
	strcpy(user.password2,pass1); 
	user.active=1;
	user.type=2;
	user.balance=0.0;
	user.acc_no=getnewAcc_no();
	create_new(user);
	msgtoclient(client,"\nJoint Account Created Succefully\n",0);

}
void AddUser(int client)
{
		char u[]="\nEnter Username : ";
		msgtoclient(client,u,1);
		char name[100];
		msgfromclient(client,name);
		int r=checkUser(name);
		char s[]="Username Already Exists..Please Enter Different Username\n";
		while(r==-1)
		{
			msgtoclient(client,s,0);
			msgtoclient(client,u,1);
			msgfromclient(client,name);
			r=checkUser(name);		
		}
		char s1[]="\nValid Username\n";
		char p[]="\nEnter Password : ";
		msgtoclient(client,p,2);
		char pass[100];
		msgfromclient(client,pass);
		char m[]="Which type of account do you wish to create ?\n1.Joint\n2.Normal\n";
		msgtoclient(client,m,1);
		char ch[100];
		msgfromclient(client,ch);
		struct Customer user;
		char abc[]="\n1.Join to an already created account\n2.Create new Joint Account\n";
		switch(ch[0])
		{
			case '1' :
			  
			  msgtoclient(client,abc,1);
			  char ch1[100];
			  msgfromclient(client,ch1);
			 // printf("%s\n",ch1);
			  if(ch1[0]=='1')
			  	Join_Existing(client,name,pass);
			  else
			  	Joint_New(client,name,pass);
			  break;
			case '2' :
				
				 strcpy(user.username1,name);
				 strcpy(user.password1,pass);
	             user.acc_no=getnewAcc_no();
	             user.type=1;
	             user.active=1;
	             user.balance=0.0;
			     create_new(user);
			     msgtoclient(client,"\nUser Account Created Successfully\n",0);
			     break;
		}
	
}

void ModifyUser(struct Customer user)
{
	struct flock lock;
	lock.l_type=F_WRLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=(user.acc_no)*sizeof(user);
	lock.l_len=sizeof(user);
	lock.l_pid=getpid();
	int fd=open("database.txt",O_RDWR);
	if(fd==-1)
	{
		perror("");
		
	}
	fcntl(fd,F_SETLKW,&lock);
	
	int flag=-1;
	
	lseek(fd,(user.acc_no)*sizeof(user),SEEK_SET);
	write(fd,&user,sizeof(user));
	lock.l_type=F_UNLCK;
	fcntl(fd,F_SETLK,&lock);
	close(fd);
}
void DeleteUser(int client)
{
	msgtoclient(client,"\nEnter Username you want to delete : ",1);
	char name[100];
	msgfromclient(client,name);
	int r=checkUser(name);
	while(r!=-1)
	{
		msgtoclient(client,"\nUsername does not exist..Enter Different username : ",1);
		msgfromclient(client,name);
		r=checkUser(name);
	}
	struct Customer user=getuser(name);
	if(user.type==2)
	{
		msgtoclient(client,"\nAre you sure you want to delete?\n1.Yes\n2.No\n",1);
		char ch[100];
		msgfromclient(client,ch);
		switch(ch[0])
		{
			case '1' :
				if(strcmp(user.username1,name)==0)
				{
					strcpy(user.username1,"");
					strcpy(user.password1,"");
				}
				else
				{
					strcpy(user.username2,"");
					strcpy(user.password2,"");
				}
				user.type=1;
				ModifyUser(user);
				msgtoclient(client,"\nUser Deleted..It is no longer a joint account\n",0);
				break;
			case '2' :
				msgtoclient(client,"\nAccount not deleted\n",0);
				break;
		}

	}
	else 
	{
		if(user.balance>0.0)
		{
			msgtoclient(client,"\nThere is balance left..Are you sure you want to delete the account?\n1.yes\n2.No\n",1);
			
		}
		else
		{
			msgtoclient(client,"\nAre you sure you want to delete?\n1.Yes\n2.No\n",1);
		}
			char ch[100];
			msgfromclient(client,ch);
			switch(ch[0])
			{
				case'1' :
					user.active=0;
					ModifyUser(user);
					msgtoclient(client,"\nAccount Deleted Successfully\n",0);
					break;
				case '2' :
					msgtoclient(client,"\nAccount not deleted\n",0);
			}

	}
}
void getBalance_Admin(int client)
{
	msgtoclient(client,"\nEnter Username : ",1);
	char name[100];
	msgfromclient(client,name);
	int r=checkUser(name);
	while(r!=-1)
	{
		msgtoclient(client,"\nUsername does not exist..Enter Different username : ",1);
		msgfromclient(client,name);
		r=checkUser(name);
	}
	struct Customer user=getuser(name);
	msgtoclient(client,"\nCurrent Available Balance is : ",0);
	char balance[20];
	sprintf(balance,"%f",user.balance);
	msgtoclient(client,balance,0);

}
void print_log_A(int client)
{
	msgtoclient(client,"\nEnter Username : ",1);
	char name[100];
	msgfromclient(client,name);
	int r=checkUser(name);
	while(r!=-1)
	{
		msgtoclient(client,"\nUsername does not exist..Enter Different username : ",1);
		msgfromclient(client,name);
		r=checkUser(name);
	}
	struct Customer user=getuser(name);
	print_log(client,user);
}

void Password_change_A(int client)
{
	msgtoclient(client,"\nEnter Username : ",1);
	char name[100];
	msgfromclient(client,name);
	int r=checkUser(name);
	while(r!=-1)
	{
		msgtoclient(client,"\nUsername does not exist..Enter Different username : ",1);
		msgfromclient(client,name);
		r=checkUser(name);
	}
	struct Customer user=getuser(name);
	if(strcmp(user.username1,name)==0)
	{
		msgtoclient(client,"\nEnter new Password : ",2);
		char pass1[100];
		msgfromclient(client,pass1);
		strcpy(user.password1,pass1);
		ModifyUser(user);
		msgtoclient(client,"\nPassword Changed Successfully\n",0);
	}
	else if(strcmp(user.username2,name)==0)
	{
		msgtoclient(client,"\nEnter new Password : ",2);
		char pass1[100];
		msgfromclient(client,pass1);
		strcpy(user.password2,pass1);
		ModifyUser(user);
		msgtoclient(client,"\nPassword Changed Successfully\n",0);
	}
}
void Password_change(int client,struct Customer user)
{
	msgtoclient(client,"\nEnter Username : ",1);
	char name[100];
	msgfromclient(client,name);
	msgtoclient(client,"\nEnter old Password : ",2);
	char pass[100];
	msgfromclient(client,pass);
	if(strcmp(user.username1,name)==0&&strcmp(user.password1,pass)==0)
	{
		msgtoclient(client,"\nEnter new Password : ",2);
		char pass1[100];
		msgfromclient(client,pass1);
		strcpy(user.password1,pass1);
		ModifyUser(user);
		msgtoclient(client,"\nPassword Changed Successfully\n",0);
	}
	else if(strcmp(user.username2,name)==0&&strcmp(user.password2,pass)==0)
	{
		msgtoclient(client,"\nEnter new Password : ",2);
		char pass1[100];
		msgfromclient(client,pass1);
		strcpy(user.password2,pass1);
		ModifyUser(user);
		msgtoclient(client,"\nPassword Changed Successfully\n",0);
	}
	else
		msgtoclient(client,"\nUsername or Password Incorrect\n",0);
}

int menu(int client,struct Customer user)
{
	int flag=0;
	if(user.type==1||user.type==2) //Normal User
	{
		char m[]="\n\nWhat do you want to do?\n1.GetInformation\n2.GetBalance\n3.Credit\n4.Debit\n5.Change Password\n6.MiniStatement\n7.Exit\n";
	
		msgtoclient(client,m,1);
		char ch[100];
		msgfromclient(client,ch);
		switch(ch[0])
		{
			case '1':
				getInfo(client,user);
				break;
			case '2':
				getBalance(client,user);
				break;
			case '3':
				Credit(client,user);
				break;
			case '4':
				Debit(client,user);
				break;
			case '5' :
				Password_change(client,user);
				break;
			case '6' :
				print_log(client,user);
				break;
			case '7':
				flag=1;
				break;
		}

		
	}


	else if(user.type==0)  //admin functions
	{
		char m1[]="\n\nWelcome Admin\nWhat do you want to do?\n1.GetInformation\n2.GetBalance\n3.AddUser\n4.DeleteUser\n5.MiniStatement\n6.Change Password\n7.Exit\n";
		msgtoclient(client,m1,1);
		char ch1[100];
		msgfromclient(client,ch1);
		switch(ch1[0])
		{
			case '1':
				getInfo_Admin(client);
				break;
			case '2':
				getBalance_Admin(client);
				break;
			case '3':
				AddUser(client);
				break;
			case '4':
				DeleteUser(client);
				break;
			case '5' :
				print_log_A(client);
				break;
			case '6' :
				Password_change_A(client);
				break;
			case '7':
				flag=1;
				break;
		}
	}
	
	return flag;

}
	
	
void init(int client)
{
	//struct Customer user;
	char name[100],pass[100]; 
	char wel[]="\n___________Welcome to the Bank_______\nEnter Username : ";
	msgtoclient(client,wel,1);
	msgfromclient(client,name);
	char p[]="Enter Password : ";
	msgtoclient(client,p,2);
	msgfromclient(client,pass);
	struct Customer user=auth(name,pass);
	char s[]="Valid User\n";
	char s1[]="Invalid Username or Password\nEnter Username: ";
	while(user.active==0)
	{
		msgtoclient(client,s1,1);
		msgfromclient(client,name);
		msgtoclient(client,p,2);
		msgfromclient(client,pass);
		user=auth(name,pass);
	}
	msgtoclient(client,s,0);
	int r=0;
	while(r!=1)
	 r=menu(client,user);	
}

	
int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 20) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);
			
			while(1){
				init(newSocket);
			}
					}

	}

	close(newSocket);


	return 0;
}
