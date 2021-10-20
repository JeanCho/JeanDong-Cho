#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>

#define MAX 10000
#define PORT 1234
#define BLKSIZE 4096
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];
int n;
int nn;
char ans[MAX];
char line[MAX];
char gpath[128];
char *dir[64];
char *arg[64];
char templine[MAX];
int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  nn = 0;
  while(s){
    arg[nn++] = s;           // token string pointers   
    s = strtok(0, " ");
  }

  arg[nn] =0;                // arg[n] = NULL pointer 
  printf("TOKENIZE DONE\n");
}



int ls_ls(char *filename)
{
  struct stat fstat, *sp;
  int re, ic;
  char sstime[64];
  sp =&fstat;
   
  if((sp->st_mode & 0xF000) == 0x8000)//reg
  {
    printf("%c",'-');
    sprintf(templine,"%c",'-');
    strcat(line,templine);
    
  }
  if((sp->st_mode & 0xF000) == 0x4000)//dir
  {
    printf("%c",'d');
    sprintf(templine,"%c",'d');
    strcat(line,templine);
  } 
  if((sp->st_mode & 0xF000) == 0xA000)//link
  {
    printf("%c",'l');
    sprintf(templine,"%c",'l');
    strcat(line,templine);
  }
  char t1[9] ="rwxrwxrwx";
  char t2[9] ="xwrxwrxwr";
  for (ic =8; ic >=0; ic--)
  {
    if(sp->st_mode & (1 << ic)) //r w x
    {
      printf("%c",t1[ic]);
      sprintf(templine,"%c",t1[ic]);
      strcat(line,templine);
    }
    else
    {
      printf("%c",t2[ic]);
      sprintf(templine,"%c",t2[ic]);
      strcat(line,templine);

    }
  }  
  
  printf("%4ld ",sp->st_nlink);
  sprintf(templine,"%4ld ",sp->st_nlink);
  strcat(line,templine);
  
  printf("%4d ",sp->st_gid);
  sprintf(templine,"%4d ",sp->st_gid);
  strcat(line,templine);
  printf("%4d ",sp->st_uid);
  sprintf(templine,"%4d ",sp->st_uid);
  strcat(line,templine);
  printf("%8ld ",sp->st_size);
  sprintf(templine,"%8ld ",sp->st_size);
  strcat(line,templine);


  strcpy(sstime,ctime(&sp->st_ctime));
  sstime[strlen(sstime)-1] =0;
  printf("%s ",sstime);
  
  sprintf(templine,"%s ",sstime);
  strcat(line,templine);
  printf("%s ",filename);
  sprintf(templine,"%s ",filename);
  strcat(line,templine);

  if((sp->st_mode & 0xF000)== 0XA000)
  {
    char buffer[MAX];
    readlink(filename, buffer ,MAX);
    printf(" -> %s", buffer);
    sprintf(templine," -> %s", buffer);
    strcat(line,templine);
  }
  strcat(line,"<p>");
  printf("<P>");





}

int lsdi(char *dirname)
{
  DIR *d;
  struct dirent *dp;
  d = opendir(dirname);
  char *filename;
  while((dp = readdir(d))!= NULL)
  {
    ls_ls(dp->d_name);
  }
  return 0;
}

int final_ls(char *dirname)
{
  struct stat mystat, *sp =&mystat;
  int h;
  char *filename, path[1024], cwd[128];
  filename ="./";
  printf("DOING LSSSSS\n");
  sprintf(templine,"DOING LS\n\n");
  strcpy(line,templine);
  
  if(dirname[0] != '\0')
  {
    filename = dirname;
  }
  if (h= lstat(filename, sp) < 0)
  {
    printf("NO FILE\n");
    strcpy(line,"NO FILE\n");
    
    return 1;
  }
  lsdi(filename);
}


void write_file(int sockfd,char *filename)
{
  int nf;
  FILE *fp;
  char buffer[BLKSIZE];

  fp = fopen(filename, "w");
  while (1) {
    nf = recv(sockfd, buffer, BLKSIZE, 0);
    if (nf <= 0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, BLKSIZE);
  }
  fclose(fp);
  return;
}
void send_file(FILE *fp, int sockfd){
  int n;
  char data[BLKSIZE] = {0};

  while(fgets(data, BLKSIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      printf("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, BLKSIZE);
  }
}


int main() 
{ 
    int sfd, cfd, len; 
    struct sockaddr_in saddr, caddr; 
    int i, length;
    char cwd[128];
    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    //saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT);
    
    printf("3. bind socket to server\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
      
    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    while(1){
       // Try to accept a client connection as descriptor newsock
       getcwd(cwd, 128);
       length = sizeof(caddr);
       cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
       if (cfd < 0){
          printf("server: accept error\n");
          exit(1);
       }

       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         char lineline[128];
         
         n = read(cfd, line, MAX);
         printf("server: read  n=%d bytes; line=[%s]\n", n, line);
         printf("!@#@!#@!#!#!@#@!#@!@!#@!#@!#\n\n");
         strcpy(lineline,line);
         tokenize(lineline);
         int ii;
        //  for (ii=0; ii<nn; ii++){  // show token strings   
        //   printf("arg[%d] = %s\n", ii, arg[ii]);
        //  }
         if(strcmp(arg[0],"ls")==0)
         {
           if(arg[1])
           {
             final_ls(arg[1]);
           }
           else
           {
             
             final_ls("./");
           }
         }
         if(strcmp(arg[0],"mkdir")==0)
        {
          int check;
          printf("WE ARE IN MKDIR!\n");
          check =mkdir(arg[1],0777);
          if(!check)
          {
            printf("Directory created %s\n",arg[1]);
            strcpy(line,"Directory created\n");
          }
          else
          {
            printf("Unable to create directory\n");
            strcpy(line,"Unable to create directory\n");

          }
        }

        if(strcmp(arg[0],"rmdir")==0)
        {
          int check;
          
          check =rmdir(arg[1]);
          if(!check)
          {
            printf("Directory removed %s\n",arg[1]);
            strcpy(line,"Directory removed");
          }
          else
          {
            printf("Unable to remove directory\n");
            strcpy(line,"Unable to remove directory");

          }
        }

        if(strcmp(arg[0],"rm")==0)
        {
          int check;
          char *p;
          struct stat st;

          if(remove(arg[1])==0)
          {
            printf("REMOVE SUCCEED \n\n");
            strcpy(line,"REMOVE SUCCEED");
          }
          else{
            printf("remove failed\n");
            strcpy(line,"remove failed");
          }
        }
        if(strcmp(arg[0],"pwd")==0)
        {
          printf("<p>cwd = %s\n", cwd);
          strcpy(line,cwd);
        }
        if(strcmp(arg[0],"cd")==0)
        {
          chdir(arg[1]);
          getcwd(cwd, 128);
          printf("<p>cwd = %s\n", cwd);
          strcpy(line,cwd);
        }
        if(strcmp(arg[0],"put")==0)
        {
          
          
          if(arg[1])
          {
            printf("put SUCCEED \n\n");
            write_file(cfd,arg[1]);
            strcpy(line,"SERVER : put SUCCEED \n");
          }
          else{
            printf("put failed\n");
            strcpy(line,"put failed \n\n");
          }
        }
        if(strcmp(arg[0],"get")==0)
        {
          
          
          if(arg[1])
          {
            FILE *fp;
            
             
            fp = fopen(arg[1], "r");
            if (fp == NULL) 
            {
              printf("ERROR in READING FILE\n");
            }
            else
            {
              send_file(fp, cfd);
              fclose(fp);
              printf("Sucessfully get filename = %s\n", arg[1]);
              strcpy(line,"SERVER : get SUCCEED \n");
            }
 

          }
          else
          {
            printf("failed\n");
            strcpy(line,"SERVER : get failed \n");
          }
          
          
        }

        

         if (n==0){
           printf("server: client died, server loops\n");
           close(cfd);
           break;
         }

         // show the line string
         
        

         strcat(line, " ECHO");

         // send the echo line to client 
         n = write(cfd, line, MAX);

         printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
         printf("server: ready for next request\n");
       }
    }
}
