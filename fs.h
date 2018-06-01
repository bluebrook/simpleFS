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

#include <iostream>
#include <sstream>
#include <string>

#include "disk.h"
#include <string.h>

#define INODE_SIZE 64
#define INODE_PER_BLOCK BLOCKSIZE/INODE_SIZE

#define FS_NUM_BLOCKS  1000
#define INODE_NUM_BLOCKS 20
#define MAX_FILES INODE_NUM_BLOCKS+1


using namespace std;
/* Inode implementation
 *
 *
 */
class Inode {
    public:
        static const int linknum = 13;
        Inode() {};
        //construct from a raw pointer
        Inode(void* data) {
            Inode * ptr = reinterpret_cast<Inode*>(data);
            fileSize = ptr->fileSize;
            flags = ptr->flags;
            owner = ptr->owner;
            for (int i = 0; i< linknum; i++) {
                pointer[i] = ptr->pointer[i];
            }
        }
        //~Inode();
        int fileSize = 0;     // file size in bytes
        int flags = 0;        // 0, unused, 1 used
        int owner = 0;
        int pointer[linknum] = {0};

        std::string toString() {
              std::stringstream ss;
              ss << "[ Flags: " << flags << " Owner: " << owner << " Size " << fileSize << " ";
              for (int i=0; i<linknum; i++) {
                  ss << "|" << pointer[i];
              }
              ss << " ]";
              return ss.str();
         }
};

class InodeBlock {
public:
    Inode node[INODE_PER_BLOCK];
    InodeBlock (){};
    InodeBlock (void* block) {
        Inode* ptr = reinterpret_cast<Inode*> (block);
        for(int i; i< INODE_PER_BLOCK; i++){
            node[i] = new Inode(ptr+i);
        }
    }
    //~InodeBlock ();
    std::string toString() {
          std::stringstream ss;
          for (int i=0; i<INODE_PER_BLOCK; i++) {
              ss << node[i].toString() << std::endl;
          }
          return ss.str();
     }
};

class SuperBlock {
public:
    int size;
    int iSize;
    int freeList;

    SuperBlock () {
        size = 0;
        iSize = 0;
        freeList = 0;
    };
    // init from a block pointer
    SuperBlock (void* block) {
        int* ptr = reinterpret_cast<int*>(block);
        size = *(ptr);
        iSize = *(ptr+1);
        freeList = *(ptr+2);
    };

    std::string toString() {
          std::stringstream ss;
          ss << "SUPERBLOCK:" << std::endl;
          ss << "Size: " << size << " Isize: " << iSize;
          ss << " freeList: ";
          ss << freeList;
          return ss.str();
     }
};

/* block that keep reference to other blocks */
class InDirectBlock {
    const static int size = BLOCKSIZE/4;
public:
    int pointer[size];
    InDirectBlock () {
        clear();
    }
    void clear (){
        for(int i=0; i< size; i++ ){
            pointer[i] = 0;
        }
    }

    std::string toString() {
        std::stringstream ss;
        ss << "InDirect Block:" << std::endl ;
        for (int i = 0; i<size; i++) {
            ss << pointer[i];
            ss << "|";
        }
        return ss.str();
    }

};

class FileDescriptor {

public:
     Inode inode;
     int inumber;
     int seekPtr;

     FileDescriptor(Inode inode, int inumber) {
         this->inode = inode;
         this->inumber = inumber;
         this->seekPtr = 0;
     }

     void setFileSize(int newSize) {
         inode.fileSize = newSize;
     }

     std::string toString() {
         std::stringstream ss;
         ss << "inode =";
         ss << inode.toString();
         ss << " inumber=";
         ss << inumber;
         ss << " seekPtr=";
         ss << seekPtr;
         return ss.str();
     }

};

/* This class keeps track of all files currently open.
 *A byte array called bitmap tells whether the specified index has a file open.
 *A fileDescriptor array fdArray contains FD objects.
 *Each FD object is an instance of the class FileDescriptor that maintains
 *information about the specified file.
 */

class FileTable {

public:
        FileDescriptor** fdArray;
        int* bitmap;

        FileTable() {
            fdArray = new FileDescriptor*[MAX_FILES];
            bitmap = new int[MAX_FILES];
            for (int i=0; i< MAX_FILES; i++) {
                bitmap[i] = 0;
            }
        }

        int allocate() {
            for(int i=0; i< MAX_FILES; i++) {
                if(bitmap[i]==0)
                    return i;
            }
            printf("Cannot open file ... filetable is full\n");
            return -1;
        }

        /* add inode, inumber into filetable */
        int add (Inode inode, int inumber, int fd) {

            if (bitmap[fd] != 0) {
                return -1;
            }

            bitmap[fd] = 1;
            fdArray[fd] = new FileDescriptor(inode, inumber);

            return 0;
        }

        void free(int fd) {
            bitmap[fd] = 0;
        }

        bool isValid(int fd) {
            if (fd < 0 || fd >= MAX_FILES) {
                printf("ERROR: Invalid file descriptor %d", fd);
                return false;
            } else if ( bitmap[fd] > 0) {
                return true;
            } else {
                return false;
            }
        }

        Inode* getInode(int fd) {
            if (bitmap[fd] == 0) return NULL;
            return &(fdArray[fd]->inode);
        }

        int getInumber(int fd) {
            if (bitmap[fd] == 0) return 0;
            return fdArray[fd]->inumber;
        }

        int getSeekPointer(int fd) {
            if (bitmap[fd] == 0) return 0;
            return fdArray[fd]->seekPtr;
        }

        int setSeekPointer(int fd, int newPointer){
            if (bitmap[fd] == 0) return 0;
            fdArray[fd]->seekPtr = newPointer;
            return 1;
        }

        int setFileSize(int fd, int newFileSize){
            if (bitmap[fd] == 0) return 0;
            fdArray[fd]->setFileSize(newFileSize);
            return newFileSize;
        }

        int getFDfromInumber(int inumber) {
            for(int i=0; i< MAX_FILES; i++) {
                if (bitmap[i] == 1)
                    if (fdArray[i]->inumber == inumber)
                        return i;
            }
            return -1;
        }

        std::string toString() {
            std::stringstream ss;
            for (int i=0; i<MAX_FILES; i++){
                //ss << "i = " << i << " bitmap[i] =  " << bitmap[i];
                if (bitmap[i] != 0) {
                    ss << "fd = " << i << " ";
                    ss << fdArray[i]->toString();
                    ss << std::endl;
                }
            }
            return ss.str();
        }
};

class FileSystem {
public:
    FileSystem();
    ~FileSystem();

    void countFreeBlock();
    int formatDisk(int size, int iSize);
    int shutdown();
    int create(int inumber);
    int inumber(int fd);

    int open(int inumber);
    int read(int fd, char* buffer, int len);
    int write(int fd, char* buffer, int len);

    int seek(int fd, int offset, int whence);

    int close(int fd);
    int del (int inumber);

    int allocateBlock();
    int insertBlock(Inode & inode, int number);
    void freeBlock(int block_nr);
    int getBlockNumber(Inode & inode, int number);
    Disk d;
    SuperBlock sb;
    FileTable ft;

};

#endif /* fs_hpp */
