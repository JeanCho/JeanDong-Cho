/************* open_close_lseek.c file **************/
#include "type.h"

extern int my_creat();
extern int bdalloc();
extern int get_block();

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int is_open(MINODE *mip)
{
    for (int i = 0; i < NFD; i++)
    {
        //if fd in use
        if (running->fd[i])
        {
            //and if this fd describes our specified file
            if (running->fd[i]->minodePtr == mip)
            {
                //and if file is already opened in non-read mode deny open request
                if (running->fd[i]->mode != O_RDONLY)
                    return -1;

                //else file can be opened for read multiple times: return index of current fd
                return i;
            }
        }
        //fd not in use, so use it
        else
            return i;
    }
    //something went wrong
    return -2;
}
//erasing everything for write
int truncate(MINODE *mip)
{
    //deallocate blocks in file's INODE

    //12 direct blocks
    for (int i = 0; i < 12; i++)
    {
        if (!mip->INODE.i_block[i])
            break;

        bdalloc(dev, mip->INODE.i_block[i]);
    }

    //indirect block; contains bno of block which contains array of 4-byte bno's
    if (mip->INODE.i_block[12])
    {
        char buf[BLKSIZE], *cp;
        get_block(dev, mip->INODE.i_block[12], buf);
        cp = buf;
        int bno;

        while (cp < buf + BLKSIZE) //casting cp as integer
        {
            bno = (int)cp;
            if (!bno)
                break;

            bdalloc(dev, bno);
            cp += sizeof(int);
        }
        bdalloc(dev, mip->INODE.i_block[12]);
    }

    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);//update time
    mip->INODE.i_size = 0;
    mip->dirty = 1;
}

int my_open(char *fName, int flags)//flags are mode
{
    int ino = getino(fName);
    if (!ino)
    {
        int r = my_creat(fName);
        if (r)
            return r;
        ino = getino(fName);
    }
    MINODE *mip = iget(dev, ino);

    //check for regular type
    if (!S_ISREG(mip->INODE.i_mode))
    {
        printf("error - my_open: %s is not a regular file.\n", fName);
        return -1;
    }

    //check if file is already opened with incompatible mode
    int fdToOpen = is_open(mip);
    if (fdToOpen == -1)
    {
        printf("error - my_open: %s is already open in an incompatible mode.\n", fName);
        return -1;
    }
    if (fdToOpen == -2)
    {
        printf("error - my_open: something went wrong.\n");
        return -2;
    }

    //must check file permissions

    //get first FREE entry (lowest index) in PROC
    OFT *oftp;//OFT = array of OFT structure  which is info about the open file
    for (int i = 0; i < NOFT; i++)
    {
        oftp = &oft[i];
        //break loop if free entry found (entry has no references)
        if (!oftp->refCount)
            break;
    }

    //check if no entries were free
    if (fdToOpen == -1)
    {
        printf("error - my_open: no free file descriptors.\n");
        return -3;
    }

    oftp->minodePtr = mip;
    //0 (RD); 1 (WR); 2 (RW); 3 (APPEND)
    oftp->mode = flags;
    oftp->refCount = 1;
    //add target file to running PROC's OFT arrary
    running->fd[fdToOpen] = oftp;

    //get mode from flags
    switch (flags)
    {
    //read mode: offset = 0, update access time
    case 0:
        oftp->offset = 0;
        mip->INODE.i_atime = time(0L);
        break;

    //write mode: clear contents of file, offset = 0, update access time
    case 1:
        truncate(mip);//deleting everything
        break;

    //read-write mode: offset = 0, update access and modification time
    case 2:
        oftp->offset = 0;
        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
        break;

    //append mode: offset = size of file, update access and modification time
    case 3:
        oftp->offset = mip->INODE.i_size;
        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
        break;

    default:
        printf("error - my_open: %d is an invalid mode.\n", flags);
        return -4;
        break;
    }

    //we changed stuff
    mip->dirty = 1;
    printf("open OK (fd = %d).\n", fdToOpen);
    return fdToOpen;
}

int my_close(int fd)//file can opened 10 many
{
    if (fd < 0 || fd > NFD)//check if proc points to whatever
    {
        printf("error - my_close: fd out of range [0-64].\n");
        return -1;
    }

    //running PROC must point at target OFT entry
    if (!running->fd[fd])
    {
        printf("error - my_close: no OFT entry pointed at by %s.\n", fd);
        return -2;
    }

    OFT *oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;//because we are closing

    //if there are references outstanding simply return
    if (oftp->refCount > 0)
    {
        printf("close OK.\n");
        return 0;
    }

    //no more references, get inode out of memory
    iput(oftp->minodePtr);
    printf("close OK.\n");
    return 0;
}

int my_lseek(int fd, int position)//goes to position in the file
{
    OFT *oftp = running->fd[fd];//oft pointer point at fd we passed in
    if (!oftp)
    {
        printf("error - my_lseek: no file described by %s.\n", fd);
        return -1;
    }

    if (position < 0)
    {
        printf("error - my_lseek: invalid position.\n");
        return -2;
    }

    int pos0 = oftp->offset;//old position of the file
    oftp->offset = position;//change to new position
    return pos0;//return old position
}