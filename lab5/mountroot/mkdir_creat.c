////level1//////////
int tst_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    if (buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] |= (1 << j);

    return 0;
}

int clr_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] &= ~(1 << j);

    return 0;
}


int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    SUPER *sp;
    GD    *gp;

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);

    return 0;
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];

    SUPER *sp;
    GD    *gp;

    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *) buf;
    sp->s_free_inodes_count++;
    
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *) buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);

    return 0;
}

int ialloc(MINODE *fs)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(fs->dev, imap, buf);

    for (i=0; i < ninodes; i++)
    {
        if (!tst_bit(buf, i))  // Inode not already allocated
        {
            set_bit(buf,i);
            decFreeInodes(fs->dev);

            put_block(fs->dev, imap, buf);

            return i+1;
        }
    }
    printf("ialloc(): no more free inodes\n");
    return 0;
}

int balloc(MINODE *fs)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(fs->dev, bmap, buf);

    for (i=0; i < nblocks; i++)
    {
        if (!tst_bit(buf, i))  // Block not already allocated
        {
            set_bit(buf,i);
            decFreeInodes(fs->dev);

            put_block(fs->dev,  bmap, buf);

            return i+1;
        }
    }
    printf("balloc(): no more free blocks\n");
    return 0;
}

void idalloc(MINODE *fs, int ino)
{
    char buf[BLKSIZE];
    if (ino > ninodes)
    {
        printf("Error: ino %d out of range\n", ino);
        return;
    }
    printf("IDLOCHIT!!!!!!!!!!!!\n");
    get_block(fs->dev, imap, buf);
    
    clr_bit(buf, ino-1);
    put_block(fs->dev, imap, buf);
    incFreeInodes(fs->dev);
}

void bdalloc(MINODE *fs, int bno)
{
    char buf[BLKSIZE];
    if(bno > nblocks)
    {
        printf("Error: ino %d out of range\n", bno);
        return;
    }
    get_block(fs->dev, bmap, buf);
    clr_bit(buf, bno-1);
    put_block(fs->dev, bmap, buf);
    incFreeInodes(fs->dev);
}

////mkdir///////////

int newdir(MINODE *pip)
{
    // Creates a new directory under pip and returns ino

    int ino = ialloc(pip);
    int bno = balloc(pip);
    char buf[BLKSIZE];

    // Allocate the new Directory
    MINODE* mip = iget(pip->dev, ino);
    INODE * ip  = &mip->INODE;

    DIR* dp;
    char *cp;

    ip->i_mode = (0x41ED);      // Directory with 0??? permissions
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = BLKSIZE;		// Size in bytes 
    ip->i_links_count = 2;	    // Links count=2 because of . and ..
    
    ip->i_mtime = time(0L);     // Set all three timestamps to current time
    ip->i_ctime = ip->i_mtime;
    ip->i_atime = ip->i_ctime;
    
    ip->i_blocks = 2;           // LINUX: Blocks count in 512-byte chunks 
    ip->i_block[0] = bno;       // new DIR has one data block   

    for (int i = 1; i < 15; i++)
    {
        ip->i_block[i] = 0;     // Set all blocks to 0
    }
    
    mip->dirty = 1;             // Set dirty for writeback

    // Initializing the newly allocated block
    get_block(mip->dev, ip->i_block[0], buf);

    dp = (DIR *) buf;
    cp = buf;

    // Create initial "." directory
    strcpy(dp->name, ".");
    dp->inode = ino;
    dp->name_len = 1;
    dp->rec_len = 12;
    
    cp += dp->rec_len;
    dp = (DIR*) cp;

    // Create initial ".." directory
    strcpy(dp->name, "..");
    dp->inode = pip->ino;
    dp->name_len = 2;
    dp->rec_len = 1012;  // Uses up the rest of the block

    // Write back initialized dir block
    put_block(mip->dev, ip->i_block[0], buf);
    
    iput(mip);
    iput(pip);

    return ino;
}


int mmkdir(char* pathname)
{
    

    
    char parent_path[128], filename[128];

    int ino, pino;
    MINODE *mip, *pip;

    // path is pathname we wanna create
    if (pathname[0] == '/')
    {
        // absolute path
        mip = root;
    }
    else
    {
        // relative path
        mip = running->cwd;
    }
    
    strcpy(parent_path, pathname);
    strcpy(filename, pathname);
    
    strcpy(parent_path, dirname(parent_path));  // "." if inserting in cwd
    strcpy(filename, basename(filename));

    pino = getino(parent_path);
    pip = iget(mip->dev, pino);

    // check if parent INODE is a dir 
    if (S_ISDIR(pip->INODE.i_mode))
    {
        // check child does not exist in parent directory
        ino = search(pip, filename);

        if (ino > 0)
        {
            printf("Child %s already exists\n", filename);
            return 1;
        }
    }
    else
    {
        printf("%s is not a dir\n", parent_path);
        return 1;
    }

    ino = newdir(pip);  // allocates a new directory
    pip->INODE.i_links_count++;

    DIR dirent;

    dirent.inode = ino;
    strncpy(dirent.name, filename, strlen(filename));
    dirent.name_len = strlen(filename);
    dirent.rec_len = ideal_len(&dirent);

    insert_entry(pip, &dirent);
    
    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;
    iput(pip);

    return 0;
}
///////creat/////////

int newfile(MINODE *fs)
{
    int ino = ialloc(fs);
    int bno = balloc(fs);

    // Allocate the new File
    MINODE* mip = iget(fs->dev, ino);
    INODE * ip  = &mip->INODE;
    //0x2F3         0x81A4

    ip->i_mode = (0x81A4);      // permissions
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = 0;		// Size in bytes 
    ip->i_links_count = 1;
    
    ip->i_mtime = time(0L);     // Set all three timestamps to current time
    ip->i_ctime = ip->i_mtime;
    ip->i_atime = ip->i_ctime;
    
    ip->i_blocks = 2;           // LINUX: Blocks count in 512-byte chunks 
    ip->i_block[0] = bno;       // new DIR has one data block   

    for (int i = 1; i < 15; i++)
    {
        ip->i_block[i] = 0;     // Set all blocks to 0
    }
    mip->dirty = 1;             // Set dirty for writeback

    iput(mip);
    
   
    return ino;
}

int mcreat(char* pathname)
{
   

    
    char parent_path[128], filename[128];

    int ino, pino;
    MINODE *mip, *pip;
    
    // path is pathname we wanna create
    if (pathname[0] == '/')
    {
        // absolute path
        mip = root;
    }
    else
    {
        ///relative path
        mip = running->cwd;
    }

    strcpy(parent_path, pathname);
    strcpy(filename, pathname);
    
    strcpy(parent_path, dirname(parent_path));  // Will be "." if inserting in cwd
    strcpy(filename, basename(filename));

    pino = getino(parent_path);
    pip = iget(mip->dev, pino);

    // checking if parent INODE is a dir 
    if (S_ISDIR(pip->INODE.i_mode))
    {
        // check child does not exist in parent directory
        ino = search(pip, filename);

        if (ino > 0)
        {
            printf("Child %s already exists\n", filename);
            return 1;
        }
    }

    ino = newfile(pip);
    
    DIR dirent;
    
    

    dirent.inode = ino;
    
    
    strncpy(dirent.name, filename, strlen(filename));
    dirent.name_len = strlen(filename);
    dirent.rec_len = ideal_len(&dirent);

    insert_entry(pip, &dirent);
    
    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;
    //printf("pip mode:  %d \n",pip->INODE.i_mode);

    //printf("permission : %d\n", pip->INODE.i_mode);

    iput(pip);
    return 0;
}