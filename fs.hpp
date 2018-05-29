//
//  fs.hpp
//  SimpleFS
//
//  Created by JackWu on 5/25/18.
//  Copyright © 2018 JackWu. All rights reserved.
//

#ifndef fs_hpp
#define fs_hpp

#include <stdio.h>

#include "disk.h"
#define INODE_SIZE 64
#define INODE_PER_BLOCK BLOCKSIZE/INODE_SIZE

#define FS_NUM_BLOCKS  100
#define INODE_NUM_BLOCKS 20

class Inode {
public:
    Inode();
    Inode(int iNumber);
    ~Inode();
    
    int fileSize;     // file size in bytes
    int flags;        // 0, unused, 1 used
    int owner;
    int direct;
};

class SuperBlock {
    int size;
    int iSize;
    int freeList;
};

class InodeBlock {
    Inode node[INODE_PER_BLOCK];
    
    InodeBlock (){
        for (int i = 0; i < INODE_PER_BLOCK; i++ ) {
            node[i] = new Inode();
        }
    }
};

class DirectBlock {
    
};

class FileSystem {
    
public:
    int formatDisk(int size, int iSize);
    int shutdown();
    int create();
    int open();
    int inumber(int fd);
    int read(int fd, void* buffer);
    int seek(fd, int offset, int whence);
    int close(int fd);
    int del (int iNumber);
    
private:
    Disk d;
    SuperBlock sb;
};



#endif /* fs_hpp */
