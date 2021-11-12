

int newsymlink(MINODE *fs, char *src)
{
    int ino = ialloc(fs);

    // Allocate the new File
    MINODE* mip = iget(fs->dev, ino);
    INODE * ip  = &mip->INODE;

    ip->i_mode = (0xA1A4);      // File with 0??? permissions
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = strlen(src);   // Set size to length of name
    ip->i_links_count = 1;	    
    
    ip->i_mtime = time(0L);     // Set all three timestamps to current time
    ip->i_ctime = ip->i_mtime;
    ip->i_atime = ip->i_ctime;
    
    ip->i_blocks = 2;           // LINUX: Blocks count in 512-byte chunks 

    char *blocks = (char *) ip->i_block;

    memcpy(blocks, src, 84);

    mip->dirty = 1;             // Set dirty for writeback

    iput(mip);

    return ino;
}

int msymlink(char* pathname, char* pathname2)
{
   
    // Copies src pathname into the i_block of an INODE
    char *src = pathname;
    char *dest = pathname2;

    char parent_path[128], filename[128];

    int ino, pino;
    MINODE *mip, *pip;

    if (dest[0] == '/')
    {
        // absolute path
        mip = root;
    }
    else
    {
        ///relative path
        mip = running->cwd;
    }

    strcpy(parent_path, dest);
    strcpy(filename, dest);
    
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
            printf("Child %s already exists!!\n", filename);
            return 1;
        }
    }

    ino = newsymlink(pip, src);
    
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

// int mreadlink(int argc, char* args[])
// {
//     if (argc < 1)
//     {
//         puts("Usage: symlink");
//         return 1;
//     }

//     // Reads symlink to print destination
//     char *pathname = args[0];

//     char link[84];

//     char parent_path[128], filename[128];

//     int ino, pino;
//     MINODE *mip, *pip, *wd;

//     if (pathname[0] == '/')
//     {
//         // absolute path
//         wd = root_fs->root;
//     }
//     else
//     {
//         ///relative path
//         wd = running->cwd;
//     }

//     strcpy(parent_path, pathname);
//     strcpy(filename, pathname);
    
//     strcpy(parent_path, dirname(parent_path));  // Will be "." if inserting in cwd
//     strcpy(filename, basename(filename));

//     pino = getino(wd, parent_path);
//     pip = iget(wd->fs, pino);

//     // checking if parent INODE is a dir 
//     if (S_ISDIR(pip->INODE.i_mode))
//     {
//         // check child does not exist in parent directory
//         ino = search(pip, filename);

//         if (ino < 0)
//         {
//             printf("Child %s already exists\n", filename);
//             return 1;
//         }
//     }

//     mip = iget(wd->fs, ino);

//     getlink(mip, link);

//     puts(link);

//     return 0;
// }