//
//  disk.cpp
//  SimpleFS
//
//  Created by JackWu on 5/25/18.
//  Copyright © 2018 JackWu. All rights reserved.
//

#include "disk.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

Disk::Disk():
filename(NULL), fd(-1),num_blocks(0),readCount(0),writeCount(0)
{
    
}

Disk::~Disk ()
{
    if(fd >=0) {
        fsync(fd);
        close(fd);
    }
    printf("Shut Down: total read %d, total write %d\n");
}

int Disk::openDisk(char *filename, int nbytes)
{
    num_blocks = (nbytes + BLOCKSIZE - 1)/BLOCKSIZE;
    //fd = open(filename, O_RDWR|O_CREAT, 0644) ;
    if((fd = open(filename, O_RDWR|O_CREAT, 0644) )== -1){
        printf("Failed to open file %s\n", filename);
        exit(1);
    }
    if (ftruncate(fd, num_blocks*BLOCKSIZE) == -1)  {
        printf("Failed to truncate file %s to size %d\n", filename, num_blocks*BLOCKSIZE);
        exit(1);
    }
       return 0;
}
int Disk::readBlock(int blocknr, void *block){
    if (blocknr > num_blocks-1) {
        printf("The blocknr = %d exceeds the disk limits %d\n", blocknr, num_blocks);
    }
    if (lseek(fd, blocknr*BLOCKSIZE, SEEK_SET) == -1) {
        printf("Failed to seek to block %d\n", blocknr);
        exit(1);
    }
    if(read(fd, block, BLOCKSIZE)==-1) {
        printf("Failed to read block %d\n", blocknr);
        exit(1);
    }
    readCount++;
    return 0;
};
int Disk::writeBlock (int blocknr, void *block){
    printf("fd =%d, current pos= %lld\n", fd, lseek(fd, 0, SEEK_CUR));
    if (blocknr > num_blocks-1) {
        printf("The blocknr = %d exceeds the disk limits %d\n", blocknr, num_blocks);
        return -1;
    }
    if (lseek(fd, blocknr*BLOCKSIZE, SEEK_SET) == -1) {
        printf("Failed to seek to block %d\n", blocknr);
        exit(1);
    }
    if(write(fd, block, BLOCKSIZE)==-1) {
        printf("Failed to write block %d\n", blocknr);
        exit(1);
    }
    writeCount++;
    return 0;
};

void Disk::syncDisk(){
    fsync(fd);
};

void Disk:zeroDisk() {
    char buffer[BLOCKSIZE] = {0};
    for (int i = 0; i < num_blocks; i++) {
        writeBlock(i, buffer);
    }
    syncDiks();
}

/*
int main(int argc, char** argv) {
    Disk d;
    char filename[20] = "singlefile";
    d.openDisk(filename, 10*BLOCKSIZE);
    
    char data[BLOCKSIZE] = "this is a test data";
     data[BLOCKSIZE-1] = 12;
    //d.writeBlock(0, data);
   for (int i =0 ; i<10; i++) {
        memset(data, 97+i, BLOCKSIZE);
        data[BLOCKSIZE-1] = 12;
        d.writeBlock(i, data);
       char output[BLOCKSIZE];
       d.readBlock(i, output);
       if (memcmp(data,output, BLOCKSIZE) !=0 )
           printf("result difference found \n");
       
    }
}*/
