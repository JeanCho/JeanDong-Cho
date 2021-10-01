/***** LAB3 base code *****/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char gpath[128];    // hold token strings 
char *arg[64];      // token string pointers
int  n;             // number of token strings
char *Barg[64];
char dpath[128];    // hold dir strings in PATH
char *dir[64];      // dir string pointers
int  ndir;          // number of dirs   
int haspipe =0;
char head[128];
char tail[128];
int nn;

int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  n = 0;
  while(s){
    arg[n++] = s;           // token string pointers   
    s = strtok(0, " ");
  }
  arg[n] =0;                // arg[n] = NULL pointer 
}



int do_command(char *cmdline,char *env[ ])
{
    char line[99];
    char cmd2[99];
    
    strcpy(cmd2,cmdline);
    tokenize(cmd2);

    strcpy(line, dir[0]); strcat(line, "/"); strcat(line, arg[0]);
    printf("command line = %s\n", line);
    tokenize(cmdline);
                    
    int r = execve(line, arg, env);
    printf("execve failed r = %d\n", r);
    exit(1);
}

int do_pipe(char * cmdline, int *pd,char *env[ ])
{
    int pid;
    if(pd)
    {
        close(pd[0]);
        dup2(pd[1],1);
        close(pd[1]);
    }
    haspipe = scan(cmdline,head,tail);
    if(haspipe)
    {
        int ipd[2];
        pid = fork();
        if(pid)
        {
            printf("if PID TAIL : %s\n\n",tail);
            close(ipd[1]);
            dup2(ipd[0],0);
            close(ipd[0]);
            do_command(tail,env);
        }
        else
        {
            printf("ELSE PID HEAD : %s\n\n",head);
            do_pipe(head,ipd,env);

        }
    }
    else
    {
        printf("NO PIPE END cmdline : %s\n",cmdline);
        do_command(cmdline,env);

    }

}

int scan(char *cmdline, char * head, char * tail)
{
    char *s;
    char ppath[128];
    char *parg[64];
    int pdetect =0;
    
    strcpy(ppath, cmdline);   // copy into global gpath[]
    s = strtok(ppath, " ");    
    nn = 0;
    while(s)
    {
    parg[nn++] = s;           // token string pointers   
    s = strtok(0, " ");
    }
    parg[nn] =0;
    parg[nn+1] =0;
    parg[nn+2] = 0;
    parg[nn+3] = 0;
    int i;
    int pp;
    for(i =nn-1; i >0;i--)
    {
         
        if(strcmp(parg[i],"|")==0&& pdetect == 0)
        {
            printf("|||||     %s\n",parg[i]);
            //for the tail
            pdetect = 1;
            strcpy(tail,parg[i+1]);
            if(parg[i+2])
            {
                printf("parg[i+2]\n");
                strcat(tail, " ");
                strcat(tail,parg[i+2]);

            }
            
            if(parg[i+3])
            {
                printf("parg[i+3]\n");
                strcat(tail," ");
                strcat(tail,parg[i+3]);
            }
            //for the head
            printf("now for the head\n");
            for(pp = 0; pp<i; pp++)
            {
                printf("pipe head[%d] = %s\n", pp, parg[pp]);
                if(pp ==0)
                {
                    strcpy(head,parg[pp]);
                    strcat(head," ");
                }
                else
                {
                    strcat(head,parg[pp]);
                    strcat(head," ");
                }

            }

        }
    }

    if(pdetect == 1)
    {
        printf("PIPE DETECTED#########\n");
        printf("HEAD : %s\n",head);
        printf("TAIL : %s\n",tail);
        return 1;
    }
    else
    {
        printf("PIPE NOT DETECTED###########\n");
        return 0;

    }



}







int main(int argc, char *argv[ ], char *env[ ])
{
  int i;
  int pid, status;
  char *cmd;
  char line[99];
  char line2[99];
  char cmdline[99];
  char *headd;
  char *taill;
  int pd[2];
  int piped =0;
  int dn =0;      
  // The base code assume only ONE dir[0] -> "/bin"
  // YOU do the general case of many dirs from PATH !!!!
  dir[0] = "/bin";
  ndir   = 1;

  // show dirs
  
  
  while(1)
  {
    for(i=0; i<ndir; i++)
    {
        printf("dir[%d] = %s\n", i, dir[i]);

    }
    
    
    printf("Welcome to Jean's SH!!!!!                  \n");
      
    printf("sh %d running\n", getpid());
   
    printf("enter a command line : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;
    strcpy(cmdline,line);
    if (line[0]==0)
        continue;
    
    tokenize(line);
    
    
    
    

    for (i=0; i<n; i++){  // show token strings   
        printf("arg[%d] = %s\n", i, arg[i]);
        if(strcmp(arg[i],"|")==0)
        {
            piped = 1;

        }
    }
    // getchar();
    
    cmd = arg[0];         // line = arg0 arg1 arg2 ... 
    

    if (strcmp(cmd, "cd")==0){
        if(strcmp(arg[1],"..")==0)
        {
            ndir--;
            dn--;

        }
        else{
            dn++;
            ndir++;
      //strcpy(dir[dn],"/");
            char dpath[99] = "/";
            strcat(dpath,arg[1]);
            dir[dn] = dpath;

        }
      
      
      chdir(arg[1]);
      
      continue;
    }

    if (strcmp(cmd, "exit")==0)
      exit(0);

    //scan(cmdline,head,tail)
    if(piped==1)
    {
        printf("PIPE HAPPENING .......\n");
        do_pipe(cmdline, pd,env);

    }
    else
    {
     
        pid = fork();
        
        if (pid)
        {
            
            
            
        
            /////////////////////////////////////////
        printf("sh %d forked a child sh %d\n", getpid(), pid);
        printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
        pid = wait(&status);
        printf("ZOMBIE child=%d exitStatus=%x\n", pid, status); 
        printf("main sh %d repeat loop\n", getpid());
        }

        else
        {
                
        
                printf("child sh %d running\n", getpid());
        
                // I/O redirections
                for (i=0; i<n; i++)
                {  
                        if (strcmp(arg[i],">")==0)
                        {
                            arg[i] = 0;
                            close(1);
                            open(arg[i+1], O_WRONLY|O_CREAT, 0644);
                            
                        }
                        else if (strcmp(arg[i],">>")==0)
                        {
                            arg[i] = 0;
                            close(1);
                            open(arg[i+1], O_WRONLY|O_APPEND, 0644);
                        }
                        else if (strcmp(arg[i],"<")==0)
                        {
                            arg[i] = 0;
                            close(0);
                            int fd = open(arg[i+1],O_RDONLY);  
                        }

                    
                }
        
        
            // make a cmd line = dir[0]/cmd for execve()
            strcpy(line, dir[0]); strcat(line, "/"); 
            
            strcat(line, cmd);

            //}
            
            printf("line = %s\n", line);

            int L = execve(line, arg, env);

            printf("execve failed r = %d\n", L);
            exit(1);
            
        }
    }  

  }

    
     
}



    
