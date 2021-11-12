/*********** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
   return 0;
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
   return 0;
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
  return n;
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk    = (ino-1)/8 + iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("ERRO: No more free minodes\n");
  
}


INODE_LOCATION mailmail(int ino)
{
    INODE_LOCATION location;
    location.block  = (ino - 1) / 8 + iblk;
    location.offset = (ino - 1) % 8;
    
    return location;
}


void iput(MINODE *mip)
{
   /* write INODE back to disk */
 /**************** NOTE ******************************
  For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
  FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

  Write YOUR code here to write INODE back to disk
 *****************************************************/
 int i, block, offset;
 char buf[BLKSIZE];
 
 INODE_LOCATION location;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;

 location = mailmail(mip->ino);

 get_block(mip->dev, location.block, buf);
    
 INODE *ip = (INODE*) buf + location.offset;
 memcpy(ip, &mip->INODE, sizeof(INODE));
 //printf("%d = iput inode mode\n\n",mip->INODE.i_mode);
 put_block(mip->dev, location.block, buf);

  return ;
 

} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     printf("%4d  %4d  %4d    %s\n", 
           dp->inode, dp->rec_len, dp->name_len, dp->name);
     if (strcmp(temp, name)==0){
        printf("found %s : ino = %d\n", temp, dp->inode);
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/')
     mip = root;
  else
     mip = running->cwd;

  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }

   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
  // WRITE YOUR code here
  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
  // mip points at a DIR minode
  // WRITE your code here: myino = ino of .  return ino of ..
  // all in i_block[0] of this DIR INODE.
}
char print_mode(u16 mode)
{
    char *mask  = "rwxrwxrwx";
    char *bmask = "---------";
    int index = 0;

    char ftype = '\0';
    
    if (S_ISDIR(mode))
        ftype = 'd';  
    else if (S_ISLNK(mode))
        ftype = 'l';
    else if (S_ISREG(mode))
        ftype = '-';
    else
        ftype = '-'; 

    printf("%c", ftype);

    for (int shift = 8; shift >= 0; shift--)
    {
        if ((mode >> shift) & 1)
        {
            printf("%c", mask[index]);
        }
        else
        {
            printf("%c", bmask[index]);
        }
        index++;
    }

    return ftype;
}

int getlink(MINODE *mip, char buf[])
{
   
    char *blocks = (char *) mip->INODE.i_block;
    memcpy(buf, blocks, 84);
    return 0;
}


int ideal_len(DIR* dirent)
{
    return 4 * ((8 + dirent->name_len + 3) / 4);
}

int insert_entry(MINODE *dir, DIR *file)
{
    // Insert dirent for file into dir's datablocks

    int blk, required, remain;
    char buf[BLKSIZE], *cp;

    DIR *dp, *last_rec, *new_rec;

    required = ideal_len(file);

    for (int i = 0; i < 12; i++)
    {
        blk = dir->INODE.i_block[i];

        if (!dir->INODE.i_block[i])
            break;

        get_block(dir->dev, blk, buf);
  
        dp = (DIR *) buf;  // Begin traversing data blocks
        cp = buf;
        
        // Find last entry in dir
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR *) cp;
        }
        last_rec = dp;

        // How many bytes remain in current block
        remain = last_rec->rec_len;

        if (remain >= required)
        {
            last_rec->rec_len = ideal_len(last_rec);
            remain -= last_rec->rec_len;
            
            cp += last_rec->rec_len;
            new_rec = (DIR *) cp;
            
            new_rec->inode = file->inode;
            new_rec->name_len = file->name_len;
            new_rec->rec_len = remain;
            strncpy(new_rec->name, file->name, file->name_len);

            put_block(dir->dev, blk, buf);
        }
        else
        {
            printf("We need to allocate another block");
            return 1;
        }
      
    }

    return 0;
}

int delete_entry(MINODE* dir, char* name)
{
    char buf[BLKSIZE];

    DIR *dp, *prev = NULL;
    char *cp;

    DIR *delete = NULL, *before_delete = NULL, *last = NULL;

    int affected_block = -1;
    
    for (int i = 0; i < 12; i++)
    {
        if (!dir->INODE.i_block[i])
        {
            printf("No more blocks %s not found\n", name);
            return 1;
        }
        
        get_block(dir->dev, dir->INODE.i_block[i], buf);

        cp = buf;
        dp = (DIR *) buf;

        while (cp < buf + BLKSIZE)
        {
            if (!strncmp(dp->name, name, dp->name_len))
            {
                // We found the dir we wanted
                before_delete = prev;
                delete = dp;
                affected_block = i;  // This is the block that will be changed
            }

            prev = dp;
            cp += dp->rec_len;       // Move to next block
            dp = (DIR *) cp;
        }
        if (affected_block != -1)
        {
            last = prev;
            break;
        }
    }

    if (delete->rec_len == BLKSIZE)  // Only entry
    {
        bdalloc(dir->mptr, dir->INODE.i_block[affected_block]);  // Boof the entire block
        
        dir->INODE.i_size -= BLKSIZE; // Decrement the block by the size of an entire block

        for (int j = affected_block; j < 11; j++)  // Scoot the next blocks over to the left by one
        {
            dir->INODE.i_block[j] =  dir->INODE.i_block[j + 1];
        }
        dir->INODE.i_block[11] = 0;
    }
    else
    {
        if (delete == last)  // Last entry in the block
        {
            // Increase size of previous entry to overwrite current dirent
            before_delete->rec_len += delete->rec_len;
        }
        else // This wasn't the last entry in the block
        {
            // Increase length of last entry by amount deleted
            last->rec_len += delete->rec_len;

            // Scoot the entire memoryspace over to squash the deleted entry
            memcpy((char *) delete, (char *) delete + delete->rec_len, (int) (buf + BLKSIZE - (char *) delete));
        }
    }
    
    put_block(dir->dev, dir->INODE.i_block[affected_block], buf);
    dir->dirty = 1;
    
    return 0;
}


int initialize_block(MINODE *fs, int bno)
{
    char buf[BLKSIZE];
    int n = BLKSIZE / sizeof(int);

    get_block(fs->dev, bno, buf);

    for (int i = 0; i < n; i++)
    {
        // Set all direct blocks living within indirect block to zero
        ((int *)buf)[i] = 0;
    }

    put_block(fs->dev, bno, buf);

    return 0;
}

int get_from_block(MINODE *fs, int bno, int index, int allocate)
{
    // Allocate a bno within a block at index and return the allocated bno
    char buf[BLKSIZE];

    get_block(fs->dev, bno, buf);

    int *pblock = &((int *) buf)[index];

    if (*pblock == 0)
    {
        if (allocate)
        {
            *pblock = balloc(fs);
            if (allocate == 2)  // Get an indirect block
            {
                initialize_block(fs, *pblock);
            }
            put_block(fs->dev, bno, buf);
        }
        else
        {
          printf("returning -1\n");
            return -1;
        }
    }
    printf("GOT PBLock %d\n",*pblock);
    return *pblock;
}






int get_ith_block(MINODE *mip, int i, int allocate)
{
    // Allocate Truthy value means function will allocate the new block for MINODE if it doesn't exist
    // Get corresponding block # for index i in order
    INODE *ip = &mip->INODE;

    int nn = BLKSIZE / sizeof(int);  // Number of bnos stored in each block
    
    int num_blocks = 12;
    int num_iblocks = nn;
    int num_diblocks = nn * nn;

    if (i < 0)
    {
        puts("Invalid block number!");
        return -1;
    }
    // DIRECT BLOCKS: 0 <= i < 11
    else if (i < num_blocks)
    {
        if (ip->i_block[i] == 0)
        {
            if (allocate)
            {
                ip->i_block[i] = balloc(mip);
            }
            else
            {
                printf("gib return -1\n");
                return -1;
            }
        }
        printf("gib return ipiblock\n");
        return ip->i_block[i];
    }
    // INDIRECT BLOCK: 12/nblocks
    else if (i < num_blocks + num_iblocks)
    {
        if (ip->i_block[12] == 0)
        {
            if (allocate)
            {
                ip->i_block[12] = balloc(mip);
                initialize_block(mip, ip->i_block[12]);
            }
            else
            {
              printf("numnum return -1\n");
                return -1;
            }
        }
        printf("gib return getfrom block\n");
        return get_from_block(mip, ip->i_block[12], i - num_blocks, allocate);
    }
    else
    {
      return 0;
    }
}
int truncate(MINODE *mip)
{
    // Deallocates all of the blocks used by inode
    int block, i = 0;

    // While there are still blocks
    while ((block = get_ith_block(mip, i, 0) != 0))
    {
        bdalloc(mip, block);
        i++;
    }

    return i;  // Return number of blocks deallocated
}
///link./././
int getdir(MINODE *mip, char *pathname)
{
    // Tries to get a valid directory from pathname
    char parent_path[256];
    int dest_ino;

    strcpy(parent_path, pathname);

    dest_ino = search(mip, pathname);

    if (dest_ino <= 0)
    {
        // Try the parent, maybe the file doesn't exist yet
        strcpy(parent_path, dirname(parent_path));
        dest_ino = search(mip, parent_path);
        printf("search again\n");
    }

    if (dest_ino < 0)
    {
        printf("%s does not exist\n", parent_path);
        return -1;
    }
    // if(dest_ino == 0)
    // {
    //     printf("%s does not exist and destino = 0\n", parent_path);
    //     return 0;
    // }
    else
    {
        // File exists: Check if it's a file or a dir
        mip = iget(mip->dev, dest_ino);
        if (!S_ISDIR(mip->INODE.i_mode))
        {
            printf("%s already exists\n", parent_path);
            return -1;
        }
        else
        {
            return dest_ino;
        }
    }
}
