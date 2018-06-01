//
//  disk.h
//  SimpleFS
//
//  Created by JackWu on 5/24/18.
//  Copyright © 2018 JackWu. All rights reserved.
//

#ifndef disk_h
#define disk_h

#define BLOCKSIZE 512

class Disk {
public:
    Disk();
    ~Disk();
    int openDisk(const char *filename, int nbytes);
    int readBlock(int blocknr, void *block);
    int writeBlock(int blocknr, void *block);
    void zeroDisk();
    void syncDisk();
private:
    char* filename;
    int fd;
    int num_blocks;
    int readCount;
    int writeCount;
};

#endif /* disk_h */
