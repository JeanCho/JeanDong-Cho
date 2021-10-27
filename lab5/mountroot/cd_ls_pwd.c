/************* cd_ls_pwd.c file **************/


MNTABLE     filesystems[NMOUNT], *root_fs, *cur_fs;




int cd(char* pathame)
{
  
    MINODE* mip = running->cwd;
    int ino = 0;

    if (pathname)
    {
        ino = getino(pathname);
        
        if (ino > 0)
        {
            mip = iget(mip->dev, ino);

            if(S_ISDIR(mip->INODE.i_mode))
            {
                iput(running->cwd);
                running->cwd = mip;
                return 0;
            }
            else
            {
                printf("%s is not a directory\n", pathname);
                return 2;
            }
        }
        else
        {
            printf("%s does not exist\n", pathname);
            return 1;
        }
    }
    else  //  cd to root
    {
        iput(running->cwd);
        running->cwd = root_fs->root;
        return 0;
    }
}

int ls_file(MINODE *mip, char *name)
{
  
  // READ Chapter 11.7.3 HOW TO ls
  char entryname[512];
  char linkname[84];
  strcpy(entryname, name);
  char filetype = print_mode(mip->INODE.i_mode);

  if (filetype == 'l')  
  {
        strcat(entryname, " -> ");
        getlink(mip, linkname);
        strcat(entryname, linkname);
  }

  printf(" %8d %s\n", mip->INODE.i_size, entryname);
  iput(mip);

  return 0;
}

// int ls_dir(MINODE *mip)
// {
  

//   char buf[BLKSIZE], temp[256];
//   DIR *dp;
//   char *cp;

//   get_block(dev, mip->INODE.i_block[0], buf);
//   dp = (DIR *)buf;
//   cp = buf;
  
//   while (cp < buf + BLKSIZE){
//      strncpy(temp, dp->name, dp->name_len);
//      temp[dp->name_len] = 0;
	
//      printf("%s  ", temp);

//      cp += dp->rec_len;
//      dp = (DIR *)cp;
//   }
//   printf("\n");
// }
//strcmp(pathname, ".")==0
 // printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  // if(!pathname || !pathname[0])
  // {
  //   ls_dir(running->cwd);
    

  // }
  // else
  // {
  //   ls_file(running->cwd,pathname);

  // }
int ls(char* pathname)
{
  char *dirname = pathname;

    int ino;
    MINODE *wd, *pip, *mip;
    DIR *dp;
    char *cp, temp[256], dbuf[BLKSIZE];

    if (!pathname || !pathname[0])
    {
        wd = running->cwd;
        ino = running->cwd->ino;
    }
    else
    {
        if (pathname[0] == '/')
        {
            wd = root;
            pathname++;
        }
        else
        {
            wd = running->cwd;
        }
        ino = getino(pathname);
    }

    pip = iget(wd->dev, ino);
    

    if (S_ISDIR(pip->INODE.i_mode))
    {
        
        for (int i = 0; i < 12; i++)
        {
            if (pip->INODE.i_block[i] == 0)
                break;

            get_block(wd->dev, pip->INODE.i_block[i], dbuf);
            dp = (DIR *) dbuf;
            cp = (char *) dbuf;

            while (cp < dbuf + BLKSIZE)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = '\0';

                if (temp[0] != '.')  // skip hidden 
                {
                    mip = iget(wd->dev, dp->inode);
                    ls_file(mip, temp);
                }

                cp += dp->rec_len;  // move to next 
                dp = (DIR *)cp;
            }
        }
    }
    else
    {
        printf("%s is not a dir\n", pathname);
        ls_file(running->cwd,pathname);
    }

    iput(pip);
    iput(mip);

    return 0;
  
}

char *pwd(MINODE *wd)
{
  int block1;
  int block2;
  
  if (wd == root){
    printf("/\n");
    return;
  }
  else
  {
    pwdpwd(wd);
  }
  printf("\n");
  return;
}

int pwdpwd(MINODE *wd)
{
    char buf[BLKSIZE], dirname[BLKSIZE];
    int MYINO, PARENTINO;

    DIR* dp;
    char* cp;

    MINODE* pip;  // Parent MINODE

    if (wd == root)
    {
        return 0;
    }
    
    // Get dir block of cwd
    get_block(wd->dev, wd->INODE.i_block[0], buf);
    dp = (DIR *) buf;
    cp = buf;
    
    // Searches through cwd for cwd and parent ino
    // TODO: Replace with search?
    while(cp < buf + BLKSIZE)
    {
        strcpy(dirname, dp->name);
        dirname[dp->name_len] = '\0';
        
        if(!strcmp(dirname, "."))
        {
            MYINO = dp->inode;
        }
        if(!strcmp(dirname, ".."))
        {
            PARENTINO = dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR*) cp;
    }

    pip = iget(wd->dev, PARENTINO);
    get_block(wd->dev, pip->INODE.i_block[0], buf);
    dp = (DIR *) buf;
    cp = buf;

    while(cp < buf + BLKSIZE)
    {
        strncpy(dirname, dp->name, dp->name_len);
        dirname[dp->name_len] = '\0';
        
        if(dp->inode == MYINO)
        {
            break;
        }

        cp += dp->rec_len;
        dp = (DIR*) cp;
    }

    pwdpwd(pip);
    iput(pip);

    // Prints this part of cwd
    printf("/%s", dirname);
    return 0;
}


