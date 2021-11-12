INODE_LOCATION mailmail2(int ino)
{
    INODE_LOCATION location;
    location.block  = (ino - 1) / 8 + iblk;
    location.offset = (ino - 1) % 8;
    
    return location;
}



////////LINK!!!!/////////////
int mlink(char* pathname, char* pathname2)
{
    
    
    char filename[256];
    int src_ino, dest_ino;
    char *src = pathname;
    char *dest = pathname2;
    char buf[BLKSIZE];
    MINODE *wd;

    if (dest[0] == '/')
    {
        wd = root;
        dest++;
    }
    else
    {
        wd = running->cwd;
    }
    
    // Get INO of destination folder
    dest_ino = getdir(wd, dest);

    if (dest_ino < 0)
    {  // No valid destination directory
        return -1;
    }
    else
    {//   // Get the filename for the destination directory
    //     if (dest_ino == search(wd, dest))
    //     {  // Dest is a dir, use the original filename
    //         strcpy(filename, src);
    //     }
    //     else
    //     {  // Dest is a file, use the new filename
    //         strcpy(filename, dest);
    //     }
        strcpy(filename, dest);
        strcpy(filename, basename(filename));
    }

    // if (dest[0] == '/')
    // {
    //     wd = root;
    //     dest++;
    // }
    // else
    // {
    //     wd = running->cwd;
    // }

    // Get INO of file to link
    src_ino = getino(src);
    
    MINODE *to_link = iget(wd->dev, src_ino);
    MINODE *dir = iget(wd->dev, dest_ino);

    // Add the link to the directory
    if (!S_ISDIR(to_link->INODE.i_mode))
    {
        DIR entry;
        entry.inode = to_link->ino;
        entry.name_len = strlen(filename);
        strcpy(entry.name, filename);
        entry.rec_len = ideal_len(&entry);

        insert_entry(dir, &entry);
        printf("HAPPPY \n");
        iput(to_link);
        iput(dir);
        INODE_LOCATION location;
        // Update the refCount in memory
        location = mailmail2(to_link->ino);

        get_block(wd->dev, location.block, buf);
        INODE *link = (INODE *) buf + location.offset;
        link->i_links_count++;
        put_block(wd->dev, location.block, buf);
        
        return 0;
    }
    else
    {
        printf("Can't create link to dir\n");
        return 1;
    }
}

////unlink////////////


int munlink(char* pathname)
{
    

    int ino, pino;

    
    char filename[128], parent_path[128];

    MINODE *wd, *mip, *pip;

    if (pathname[0] == '/')
    {
        wd = root;
        pathname++;
    }
    else
    {
        wd = running->cwd;
    }

    strcpy(parent_path, pathname);
    strcpy(parent_path, dirname(parent_path));

    strcpy(filename, pathname);
    strcpy(filename, basename(filename));

    ino = getino(pathname);
    mip = iget(wd->dev, ino);

    pino = getino(parent_path);
    pip = iget(wd->dev, pino);

    if (S_ISDIR(mip->INODE.i_mode))
    {
        printf("Can't unlink: %s is a dir\n", pathname);
        return 1;
    }
    else
    {
        mip->INODE.i_links_count--;
        if (mip->INODE.i_links_count == 0)
        {
            if (S_ISREG(mip->INODE.i_mode))
            {
                truncate(mip);
            }
            else if (S_ISLNK(mip->INODE.i_mode))
            {
                // I don't think we need to do anything
            }
            idalloc(mip, mip->ino);
        }
        
        // Remove the dirent from the parent
        delete_entry(pip, filename);
    }
    
    iput(mip);
    iput(pip);

    return 0;
}