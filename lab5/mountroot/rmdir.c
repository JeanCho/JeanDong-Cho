////rmdir////

int mrmdir(char* pathname)
{
    

    
    char parent_path[128], filename[128];

    int ino, pino;
    MINODE * mip, *wd;

    if (pathname[0] == '/')
    {
        wd = root;
        
    }
    else
    {
        wd =running->cwd;
    }

    strcpy(parent_path, pathname);
    strcpy(filename, pathname);

    strcpy(parent_path, dirname(parent_path));
    strcpy(filename, basename(filename));
    
    ino = getino(pathname);

    if (ino < 0)
    {
        printf("%s not found!\n", pathname);
        return 1;
    }

    mip = iget(wd->dev, ino);

    if (running->uid != mip->INODE.i_uid && running->uid != 0)
    {
        printf("Cannot rmdir: permission denied\n");
        return 0;
    }
    else if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Cannot rmdir: %s is not a dir\n", pathname);
        iput(mip);
        return -1;
    }
    else if (mip->INODE.i_links_count > 2)
    {
        printf("Cannot rmdir: %s is not empty\n", pathname);
        iput(mip);
        return -1;
    }
    else if (mip->INODE.i_links_count == 2)
    {
        char buf[BLKSIZE];
        DIR* dp;
        char* cp;
        printf("HIT!!!!!!!!!!!!\n");
        get_block(wd->dev, mip->INODE.i_block[0], buf);
        
        cp = buf;
        dp = (DIR*) buf;

        cp += dp->rec_len;  // Get second entry in block
        dp = (DIR*) cp;
        ;
        // Second block of an empty disk will have rec_len of 1012
        if (dp->rec_len != 1012)
        {
            printf("Dir is not empty. Cannot be removed\n");
            iput(mip);
            return -1;
        }
    }
    printf("HIT!!!!!!!!!!!!\n");
    truncate(mip);
    
    idalloc(mip, mip->ino);
    
    iput(mip);
    
    // This will succeed because the getino for the child succeeded
    pino = getino(parent_path);
    
    MINODE* pip = iget(wd->dev, pino);
    delete_entry(pip, filename);
    pip->INODE.i_links_count--;  // We just lost ".." from the deleted child
    pip->INODE.i_mtime = time(0L);
    pip->INODE.i_atime = pip->INODE.i_mtime;
    pip->dirty = 1;
    iput(pip);

    return 0;
}
