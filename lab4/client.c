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
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 10000
#define PORT 1234

#define BLKSIZE 4096
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];
int nn;
char ans[MAX];
char line[MAX];
char gpath[128];
char *dir[64];
char *arg[64];

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
  return 1;
}

int n;

struct sockaddr_in saddr; 
int sfd;



//copy file1 to file2
// int copyfiles(char* file1, char* file2)
// {
//     int fd,gd,nf, total=0;
//     char buf[BLKSIZE];
//     if((fd = (open(file1, O_RDONLY))<0))
//     {
//       return 0;
//     }
//     if((gd = (open(file2, O_WRONLY|O_CREAT))<0))
//     {
//       return 0;
//     }
//     while( n= read(fd, buf,BLKSIZE))
//     {
//       write(gd,buf,nf);
//       total += nf;
//     }
//     printf("Total bytes copied=%d\n",total);
//     closde(fd);
//     close(gd);
//     return 1;
// }


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

int ls_ls(char *filename)
{
  struct stat fstat, *sp;
  int re, ic;
  char sstime[64];
  sp =&fstat;
   
  if((sp->st_mode & 0xF000) == 0x8000)//reg
  {
    printf("%c",'-');
    
  }
  if((sp->st_mode & 0xF000) == 0x4000)//dir
  {
    printf("%c",'d');
    
  } 
  if((sp->st_mode & 0xF000) == 0xA000)//link
  {
    printf("%c",'l');
    
  }
  char t1[] ="rwxrwxrwx";
  char t2[] ="xwrxwrxwr";
  for (ic =8; ic >=0; ic--)
  {
    if(sp->st_mode & (1 << ic)) //r w x
    {
      printf("%c",t1[ic]);
     
    }
    else
    {
      printf("%c",t2[ic]);
      

    }
  }  
  printf("D11111\n");  
  printf("%4ld ",sp->st_nlink);
  
  printf("%4d ",sp->st_gid);
  
  printf("%4d ",sp->st_uid);
  
  printf("%8ld ",sp->st_size);
  


  strcpy(sstime,ctime(&sp->st_ctime));
  sstime[strlen(sstime)-1] =0;
  printf("%s ",sstime);
  
  
  printf("%s ",filename);
  

  if((sp->st_mode & 0xF000)== 0XA000)
  {
    char buffer[MAX];
    readlink(filename, buffer ,MAX);
    printf(" -> %s", buffer);
    
  }
  
  printf("<P>");
  return 1;





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
  strcpy(filename ,"./");
  
  
  
  if(dirname[0] != '\0')
  {
    filename = dirname;
  }
  if (h= lstat(filename, sp) < 0)
  {
    printf("NO FILE\n");
    
    
    return 1;
  }
  lsdi(filename);
  return 0;
}






int main(int argc, char *argv[], char *env[]) 
{ 
    int n; char how[64];
    int i;
    char cwd[128];
    getcwd(cwd, 128);

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 

    printf("********  processing loop  *********\n");
    while (1){
      printf("input a line : ");
      bzero(line, MAX);                // zero out line[ ]
      fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

      line[strlen(line)-1] = 0;        // kill \n at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
      char lineline[128];
      strcpy(lineline,line);
      tokenize(lineline);
      // Send ENTIRE line to server
      n = write(sfd, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      if(strcmp(arg[0],"lpwd")==0)
        {
          printf("<p>cwd = %s\n", cwd);
          
        }
        if(strcmp(arg[0],"cd")==0)
        {
          chdir(arg[1]);
          getcwd(cwd, 128);
          printf("<p>cwd = %s\n", cwd);
          
        }
      if(strcmp(arg[0],"lls")==0)
         {
           if(arg[1])
           {
             final_ls(arg[1]);
           }
           else
           {
             char* empty;
             final_ls(empty);
           }
         }
         if(strcmp(arg[0],"lmkdir")==0)
        {
          int check;
          printf("WE ARE IN MKDIR!\n");
          check =mkdir(arg[1],0777);
          if(!check)
          {
            printf("Directory created %s\n",arg[1]);
            
          }
          else
          {
            printf("Unable to create directory\n");
            

          }
        }

        if(strcmp(arg[0],"lrmdir")==0)
        {
          int check;
          
          check =rmdir(arg[1]);
          if(!check)
          {
            printf("Directory removed %s\n",arg[1]);
            
          }
          else
          {
            printf("Unable to remove directory\n");
            

          }
        }

        if(strcmp(arg[0],"lrm")==0)
        {
          int check;
          char *p;
          struct stat st;

          if(remove(arg[1])==0)
          {
            printf("REMOVE SUCCEED \n\n");
            
          }
          else{
            printf("remove failed\n");
            
          }
        }
        if(strcmp(arg[0],"lcwd")==0)
        {
          printf("<p>cwd = %s\n", cwd);
          
        }
        if(strcmp(arg[0],"lcd")==0)
        {
          chdir(arg[1]);
          getcwd(cwd, 128);
          printf("<p>cwd = %s\n", cwd);
          
        }
       

        if(strcmp(arg[0],"lcat")==0)
        {
          int fd;
          int n;
          char buf[4096];
          strcpy(line,"\n");
          if(nn <2)
          {
            exit(1);
          }
          fd = open(arg[1],O_RDONLY);
          if(fd<0) 
          {
            exit(2);
          }
          while (n = read(fd,buf,4096))
          {
            for(i=0; i<n; i++)
            {
              write(1, &buf[i],1);
              
            }
          }

        }


        if(strcmp(arg[0],"put")==0)
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
              send_file(fp, sfd);
              fclose(fp);
              printf("Sucessfully put filename = %s\n", arg[1]);
            }
 

          }
          else
          {
            printf("failed\n");
          }
          
          
        }

        if(strcmp(arg[0],"get")==0)
        {
          
          
          if(arg[1])
          {
            
            write_file(sfd,arg[1]);
            printf("get SUCCEED \n\n");
          }
          else{
            printf("get failed\n");
            
          }
        }

      // Read a line from sock and show it
      n = read(sfd, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
  }
}

