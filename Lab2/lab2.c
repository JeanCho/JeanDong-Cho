#include <stdio.h>             // for I/O
#include <stdlib.h>            // for I/O
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node{
         char  name[64];       // node's name string
         char  type;           // 'D' for DIR; 'F' for file
   struct node *child, *sibling, *parent;
}NODE;


NODE *root, *cwd, *start;
char line[128];
char command[16], pathname[64];

//               0       1      2      3       4      5       6     
char *cmd[] = {"mkdir", "ls", "quit","tree","creat","rmdir","rm","cd", 0};

int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

NODE *search_child(NODE *parent, char *name)
{
  NODE *p;
  printf("search for %s in parent DIR  %s\n", name,parent->name);
  p = parent->child;
  if (p==0)
    return 0;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->sibling;
  }
  return 0;
}

NODE *goto_dir(NODE *parent, char *pathname)
{
  NODE *p;
  char *token;
  char* tlist[999];
  int n=0;
  char firstborn = pathname[0];
  token = strtok(pathname, "/");
  while(token != NULL)
  {
    tlist[n] = token;
    n+=1;
    token = strtok(NULL, "/");
  }
  printf("Token size N : %d\n",n);
  printf("going for %s in parent DIR\n", pathname);
  p = parent->child;
  if (p==0)
    return 0;
  int i =0;

  if(strcmp(tlist[i],"..")==0)
  {
    while(i != n-1)
    {
      cwd = cwd->parent;

    }
    return cwd;

  }
  if (firstborn == '/')
  {
    
    printf("GO to dir parent == root \n");

    while(p)
    {
      printf("pname : %s    pathname: %s\n", p->name, tlist[i]);
      
      if (strcmp(p->name, tlist[i]) == 0)
      {
        printf("HIT\n");
        printf("I    :   %d\n",i);
        if (i+2 >= n)
        {
          printf("RETURNING CHILD %s\n", p->name);
          return p;
        }
          
        else
        {
          i+=1;
          p= p->child;
          printf("WENT TO CHid %s \n",p->name);

        }
          
      }
      else
      {
        p = p->sibling;
      }
        
    }
  }
  else
  {
    printf("GO to dir parent == cwd \n");
    while(p)
    {
      printf("pname : %s    pathname: %s\n", p->name, tlist[i]);
      
      if (strcmp(p->name, tlist[i]) == 0)
      {
        printf("HIT\n");
        if (i+2 >= n)
        {
          printf("RETURNING CHILD %s\n", p->name);
          return p;
        }
          
        else
        {
          i+=1;
          p= p->child;
          printf("WENT TO CHid %s \n",p->name);

        }
          
      }
      else
      {
        p = p->sibling;
      }
        
    }
  }
  
  return 0;
}

//for ls
NODE *goto_dir2(NODE *parent, char *pathname)
{
  NODE *p;
  char *token;
  char* tlist[999];
  int n=0;
  char firstborn = pathname[0];
  token = strtok(pathname, "/");
  while(token != NULL)
  {
    tlist[n] = token;
    n+=1;
    token = strtok(NULL, "/");
  }
  printf("Token size N : %d\n",n);
  printf("going for %s in parent DIR\n", pathname);
  p = parent->child;
  if (p==0)
    return 0;
  int i =0;
  if (firstborn == '/')
  {
    
    printf("GO to dir parent == root \n");

    while(p)
    {
      printf("pname : %s    pathname: %s\n", p->name, tlist[i]);
      
      if (strcmp(p->name, tlist[i]) == 0)
      {
        printf("HIT\n");
        printf("I    :   %d\n",i);
        if (i+1 >= n)
        {
          printf("RETURNING CHILD %s\n", p->name);
          return p;
        }
          
        else
        {
          i+=1;
          p= p->child;
          printf("WENT TO CHid %s \n",p->name);

        }
          
      }
      else
      {
        p = p->sibling;
      }
        
    }
  }
  else
  {
    printf("GO to dir parent == cwd \n");
    while(p)
    {
      printf("pname : %s    pathname: %s\n", p->name, tlist[i]);
      
      if (strcmp(p->name, tlist[i]) == 0)
      {
        printf("HIT\n");
        printf("I    :   %d\n",i);
        if (i+1 >= n)
        {
          printf("RETURNING CHILD %s\n", p->name);
          return p;
        }
          
        else
        {
          i+=1;
          p= p->child;
          printf("WENT TO CHid %s \n",p->name);

        }
          
      }
      else
      {
        p = p->sibling;
      }
        
    }
  }
  
  return 0;
}

int insert_child(NODE *parent, NODE *q)
{
  NODE *p;
  printf("insert NODE %s into END of parent child list\n", q->name);
  p = parent->child;
  if (p==0)
    parent->child = q;
  else{
    while(p->sibling)
      p = p->sibling;
    p->sibling = q;
  }
  q->parent = parent;
  q->child = 0;
  q->sibling = 0;
}

int rmv(char* pathname)
{
  NODE* q;
  NODE* beginp;

  printf("-------------------------\n");
  printf("Removind direcotry %s \n", pathname);
  if(pathname[0]== '/')
  {
    beginp = root;
  }
  else
  {
    beginp = cwd;
  }
  q = goto_dir(beginp,pathname);
  printf("Went to %s node\n",q->name);
  remove_directory(q);

}


int remove_directory(NODE *q)
{
  NODE *p;
  NODE *s;
  NODE *parent;
  parent = q->parent;
  if(parent != 0)
  {
    p = q->parent->child;

  }
  
  
  if(q->child)
  {
    printf("FAILED %s not empty\n", q->name);
    return 1;
  }


  if(p == q)
  {
    p == q->sibling;
    return 1;
  }
  else
  {
    while(p->sibling)
    {
      if(p->sibling == q)
      {
        p->sibling = p->sibling->sibling;
        return 1;
        
      }
      p = p->sibling;
    }

  }
  

    
    

}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *pathname)
{
  NODE *p, *q;
  char *token;
  char* tlist[999];
  int n=0;
  char pp[99];
  char pp2[99];
  char pp3[99];
  strcpy(pp, pathname);
  strcpy(pp2, pathname);
  strcpy(pp3, pathname);
 
  printf("mkdir: name=%s\n", pathname);

  if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, "..")){
    printf("can't mkdir with %s\n", pathname);
    return -1;
  }
  if (pathname[0]=='/')
    start = root;
  else
    start = cwd;

  token = strtok(pathname, "/");
  while(token != NULL)
  {
    tlist[n] = token;
    n+=1;
    token = strtok(NULL, "/");
  }
  printf("N     :    %d\n",n);
  if(n <2)
  {
    printf("check whether %s already exists\n", pp);
    if (pp[0] == '/')
    {
      memmove(pp, pp+1, strlen(pp));

    }
    
    p = search_child(start, pp);
    if (p)
    {
      printf("name %s already exists, mkdir FAILED\n", pp2);
      return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to mkdir %s\n", pp2);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'D';
    strcpy(q->name, tlist[n-1]);
    insert_child(start, q);
    printf("mkdir %s OK\n", pp2);
    printf("--------------------------------------\n");
    return 0;

  }
  
  else
  {
    printf("check whether %s already existS\n", pp);
    start = goto_dir(start, pp);
    if (pp[0] == '/')
    {
      memmove(pp, pp+1, strlen(pp));

    }
    p = search_child(start, tlist[n-1]);
    if (p)
    {
      printf("name %s already exists, mkdir FAILED\n", pp2);
      return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to mkdir %s\n", pp2);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'D';
    strcpy(q->name, tlist[n-1]);
    
    insert_child(start, q);
    printf("mkdir %s OK\n", pp2);
    printf("--------------------------------------\n");
    
    
      
    return 0;
  }


}




int creat(char *pathname)
{
  NODE *p, *q;
  char *token;
  char* tlist[999];
  int n=0;
  char pp[99];
  char pp2[99];
  char pp3[99];
  strcpy(pp, pathname);
  strcpy(pp2, pathname);
  strcpy(pp3, pathname);
 
  printf("creat: name=%s\n", pathname);

  if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, "..")){
    printf("can't creat with %s\n", pathname);
    return -1;
  }
  if (pathname[0]=='/')
    start = root;
  else
    start = cwd;

  token = strtok(pathname, "/");
  while(token != NULL)
  {
    tlist[n] = token;
    n+=1;
    token = strtok(NULL, "/");
  }
  printf("N     :    %d\n",n);
  if(n <2)
  {
    printf("check whether %s already exists\n", pp);
    if (pp[0] == '/')
    {
      memmove(pp, pp+1, strlen(pp));

    }
    p = search_child(start, tlist[n-1]);
    if (p)
    {
      printf("name %s already exists, creat FAILED\n", pp2);
      return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to creat %s\n", pp2);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'F';
    strcpy(q->name, tlist[n-1]);
    insert_child(start, q);
    printf("creat %s OK\n", pp2);
    printf("--------------------------------------\n");
    return 0;

  }
  
  else
  {
    printf("check whether %s already existS\n", pp);
    if (pp[0] == '/')
    {
      memmove(pp, pp+1, strlen(pp));

    }
    start = goto_dir(start, pp);
    p = search_child(start, tlist[n-1]);
    if (p)
    {
      printf("name %s already exists, creat FAILED\n", pp2);
      return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to creat %s\n", pp2);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'F';
    strcpy(q->name, tlist[n-1]);
    
    insert_child(start, q);
    printf("creat %s OK\n", pp2);
    printf("--------------------------------------\n");
    
    
      
    return 0;
  }


}
  
// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char *pathname)
{
  if(strcmp(pathname, ".")==0)
  {
    NODE *p = cwd->child;
    printf("cwd contents = ");
    while(p){
      printf("[%c %s] ", p->type, p->name);
      p = p->sibling;
    }
    printf("\n");
  }
  else
  {
    if (pathname[0]=='/')
      start = root;
    else
      start = cwd;
    
    start = goto_dir2(start, pathname);
    start = start->child;
    if(strlen(pathname) >2)
    {
      start = start->child;
    }
    while(start){
      printf("[%c %s] \n", start->type, start->name);
      start = start->sibling;
    }
    


  }

}

int Preorder(NODE* spoint, char* line, FILE *fp)
{
  char lcpy[2020];
  char tcpy[2020];
  char scpy[2020];
  char *token;
  char* tlist[999];
  int n=0;
  int i =0;
  if(spoint == 0)
  {
      
    return 1;
  }
  
  strcpy(lcpy,line);
  
  if(spoint->parent == root)
  {
    strcpy(scpy,"/");
  }
  
  else
  {
    strcpy(tcpy,line);
    token = strtok(tcpy, "/");
    while(token != NULL)
    {
      tlist[n] = token;
      n+=1;
      token = strtok(NULL, "/");
    }
    strcat(scpy,"/");
    while(i != n-1)
    {
      strcat(scpy,tlist[i]);
      strcat(scpy,"/");
      i++;
    }

  }
  
  if(spoint != root )
  {
    if(spoint->parent != root)
    {
        strcat(lcpy,"/");
    }

  }
  strcat(lcpy,spoint->name);
  
 
  printf("[%c,%s]\n", spoint->type, lcpy);
  fprintf(fp,"%c:%s\n", spoint->type,lcpy);

  Preorder(spoint->child,lcpy,fp);
  
  Preorder(spoint->sibling,scpy,fp);

}


  

int quit(FILE* fp)
{
  char link[2020];
  printf("Program exit\n");
  
  Preorder(root,link,fp);
  fclose(fp);
  
  // improve quit() to SAVE the current tree as a Linux file
  // for reload the file to reconstruct the original tree
  // FILE *fp = fopen("myfile", "w+");
  // fprintf(fp,"");
  








  exit(0);
}
int restart(FILE* fp)
{
  char* token;
  char buff[255];
  char data[255];
  char* tlist[3];
  int n;
  int size = 0;
  if(fp!=NULL)
  {
      fseek (fp, 0, SEEK_END);

      size = ftell (fp);
      rewind(fp);

  }
  if (size==0)
  {
    fclose(fp);
    return 1;

  }
  while(feof(fp)==0)
  {
    n=0;
    fscanf(fp, "%s", buff);
    printf(" %s\n", buff );
    strcpy(data,buff);
    token = strtok(data, ":");
    while(token != NULL)
    {
      
      tlist[n] = token;
      n+=1;
      token = strtok(NULL, ":");
    }
    if(strcmp(tlist[0],"F")==0)
    {
      creat(tlist[1]);
    }
    else 
    {
      if(strcmp(tlist[1],"/")!=0)
      {
        mkdir(tlist[1]);

      }
      

    }
    
  }
  fclose(fp);
  
}

int initialize()
{
    FILE *fp = fopen("myfile", "r+");

    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
    restart(fp);
}


int changeDirectory(char* pathname)
{
  if(strcmp(pathname, ".")==0)
  {
    printf("Nothing happend\n");
    return 1;
  }
  else
  {
    if (pathname[0]=='/')
      start = root;
    else
      start = cwd;
    
    start = goto_dir(start, pathname);
    cwd = start;
    if(pathname[1]=='.')
    {
      printf("cwd =%s\n",cwd->name);
    }
    printf("cwd = %s\n",pathname);


  }

}
int main()
{
  int index;
  
  
  char link[2020];
  initialize();
  FILE *fp = fopen("myfile", "w+");
  printf("NOTE: commands = [mkdir|ls|quit|tree|creat|rmdir|rm|cd]\n");
  cwd = root;
  while(1){
      printf("Enter command line : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      command[0] = pathname[0] = 0;
      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      if (command[0]==0) 
         continue;

      index = findCmd(command);

      switch (index){
        case 0: mkdir(pathname); break;
        case 1: ls(pathname);            break;
        case 2: quit(fp);          break;
        case 3:Preorder(root,link,fp); break;
        case 4:creat(pathname); break;
        case 5:rmv(pathname); break;
        case 6:rmv(pathname); break;
        case 7:changeDirectory(pathname); break;
      }
  }
}

