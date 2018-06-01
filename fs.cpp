//
//  fs.cpp
//  SimpleFS
//
//  Created by Jack Wu on 5/25/18.
//  Copyright © 2018 JackWu. All rights reserved.
//

#include "fs.h"

/* Initialize SuperBlock(block_nr=0) and 20 InodeBlocks(block_nr= [1,20]),
 * The freeList is set to the blocks 21.
 */

FileSystem::FileSystem() {
    const char * filename = "Disk";
    d.openDisk(filename, FS_NUM_BLOCKS * BLOCKSIZE);
    char block[BLOCKSIZE];
    d.readBlock(0, block);
    sb = SuperBlock(block);
    std::cout << "Loading file system: " << std::endl;
    std::cout  << sb.toString() << std::endl;
    countFreeBlock();
}

void FileSystem::countFreeBlock() {
    int ptr = sb.freeList;
    char block[BLOCKSIZE];
    int ct = 0;
    while(ptr >0 && ptr !=1000 ) {
        d.readBlock(ptr, block);
        InDirectBlock *ib = reinterpret_cast<InDirectBlock*> (block);
        ptr = ib->pointer[0];
        ct++;
    }
    std::cout << "\nTotal number of Free Blocks: " << ct << std::endl;
}


FileSystem::~FileSystem() {
    d.writeBlock(0, &sb);
}

// format the disk
int FileSystem::formatDisk (int size, int iSize) {
    const char * filename = "Disk";
    d.openDisk(filename, size * BLOCKSIZE);
    d.zeroDisk();
    
    // write SuperBlock
    sb.size = size;
    sb.iSize = iSize;
    sb.freeList = iSize+1;
    d.writeBlock(0, (void *) &sb);

    // write InodeBlock
    InodeBlock ib;
    for(int i = 0; i < INODE_NUM_BLOCKS; i++) {
        d.writeBlock(i+1, &ib);
    }

    // link the freeList
    InDirectBlock indb;
    for(int i= iSize+1; i<size; i++){
        if (i < size-1 )
            indb.pointer[0] = i+1;
        else
            indb.pointer[0] = 0;
        d.writeBlock(i, &indb);
    }
    d.syncDisk();
    countFreeBlock();

    return 0;
}

// Create a new (empty) file and return a file descriptor.
int FileSystem::create(int inumber) {
    char block[BLOCKSIZE];

    for (int i=1; i<= sb.iSize; i++) {
           d.readBlock(i, (void*) block);
           InodeBlock* ib = reinterpret_cast<InodeBlock*> (block);
           for (int j =0 ; j < INODE_PER_BLOCK; j++){
               Inode & inode = ib->node[i];
               if(inode.flags == 1 && inode.owner == inumber )
               {
                   printf ("inumber = %d file already exists\n", inumber);
                   return -1;
               }
           }
       }

    for (int i=1; i<= sb.iSize; i++) {
        d.readBlock(i, (void*) block);
        InodeBlock* ib = reinterpret_cast<InodeBlock*> (block);
        for (int j =0 ; j < INODE_PER_BLOCK; j++){
            Inode & inode = ib->node[i];
            if(inode.flags == 0)
            {
                int fileDescr = ft.allocate();
                if (fileDescr >= 0)
                {
                   inode.flags = 1;
                   inode.owner = inumber;
                   inode.fileSize = 0;
                   for (int k=0; k<inode.linknum; k++) {
                       inode.pointer[k] = 0;
                   }
                   d.writeBlock(i, ib);
                   ft.add(inode, inumber, fileDescr);
                }
                return fileDescr;
            }
        }
    }
    printf("No inode for a new file");
    return -1;
}

// Return the inumber of an open file
 int FileSystem::inumber(int fd)
  {
    return ft.getInumber(fd);
  }


// Open an existing file identified by its inumber
int FileSystem::open(int iNumber) {
    for (int i=1; i< MAX_FILES; i++) {
        if (ft.bitmap[i] != 0)
            if (ft.fdArray[i]->inumber == iNumber)
                return -1;
    }

    char block[BLOCKSIZE];
    for (int i=1; i<= sb.iSize; i++) {
          d.readBlock(i, block);
          InodeBlock* ib = reinterpret_cast<InodeBlock*>(block);
          for (int j = 0; j<INODE_PER_BLOCK ; j++) {
              Inode & inode = ib->node[j];
              if (inode.flags != 0 && inode.owner == iNumber) { //file exists
                  int ind = ft.allocate();
                  if (ind >= 0) {
                    ft.add(inode, iNumber, ind);
                    return ind;
                  }
                 printf("Too many open files\n");
                 return -1;
              }
          }
    }
    printf("File doesn't exist\n");
    return -1;
}

  // allocate a block from free list
  int FileSystem::allocateBlock(){
      int cur = sb.freeList;
      char* block = new char[BLOCKSIZE];
      while(cur !=0 ) {
          d.readBlock(cur, block);
          InDirectBlock* ind_block = reinterpret_cast<InDirectBlock* >(block);
          sb.freeList = ind_block->pointer[0];
          ind_block->clear();

          d.writeBlock(cur, ind_block); // write the change back to disk
          d.writeBlock(0, &sb); // write the superBlock back

          return cur;
      }
      printf("No free Block left!\n");
      return -1;
  }

  // return a block to free list
  void FileSystem::freeBlock(int block_nr){
      int cur = sb.freeList;
      char* block = new char[BLOCKSIZE];
      d.readBlock(block_nr, block);
      InDirectBlock* ind_block = reinterpret_cast<InDirectBlock* >(block);

      ind_block->pointer[0] = sb.freeList;
      d.writeBlock(block_nr, block); // write the change to freed block

      sb.freeList = block_nr; // write SuperBlock

  }

  // insert a number block for a inode
  int FileSystem::insertBlock(Inode & inode, int number) {
      int block_nr = allocateBlock();
      int N = BLOCKSIZE/4;
      if (number < 0 || block_nr <=0 )
          return -1;

      if (number < 10) {
          inode.pointer[number] = block_nr;
          return block_nr;
      } else if (number < 10 + N ) {
          if (inode.pointer[10] == 0) {
              int ind_block_nr = allocateBlock();
              if (ind_block_nr == -1) {
                  return -1;
              }
              inode.pointer[10] = ind_block_nr;
          }
          InDirectBlock ind_block;
          d.readBlock(inode.pointer[10], &ind_block);

          ind_block.pointer[number-10] = block_nr;
          d.writeBlock(inode.pointer[10], &ind_block);
          return block_nr;
      } else if (number < 10 + N + N * N ) {
          if (inode.pointer[11] == 0) {
              int ind_block_nr = allocateBlock();
              if (ind_block_nr == -1) {
                  return -1;
              }
              inode.pointer[11] = ind_block_nr;
          }

          InDirectBlock ind_block;
          d.readBlock(inode.pointer[11], &ind_block);

          int index = (number-10-N)/N;
          int rmd = (number-10-N) % N;
          if (ind_block.pointer[index] == 0) {
              int ind_block_nr = allocateBlock();
              if (ind_block_nr == -1) {
                  return -1;
              }
              ind_block.pointer[index] = ind_block_nr;
              d.writeBlock(inode.pointer[11], &ind_block);
          }

          InDirectBlock ind_block2;
          d.readBlock(ind_block.pointer[index], &ind_block2);
          ind_block2.pointer[rmd] = block_nr;
          d.writeBlock(ind_block.pointer[index], &ind_block2);

          return block_nr;
      } else if (number < 10 + N + N * N ) {
          printf ("Too big to support in current fs\n");
          return -1;
      } else {
          printf ("Too big to support in current fs\n");
          return -1;
      }

  }

  int FileSystem::getBlockNumber(Inode & inode, int number) {
      int N = BLOCKSIZE/4;
        if (number < 0 )
            return -1;

        if (number < 10) {
            return inode.pointer[number];
        } else if (number < 10 + N ) {
            if (inode.pointer[10] == 0) {
                //shouldn't happen
                return -1;
            }
            InDirectBlock ind_block;
            d.readBlock(inode.pointer[10], &ind_block);
            return ind_block.pointer[number-10];

        } else if (number < 10 + N + N * N ) {
            if (inode.pointer[11] == 0) {
                return -1;
            }

            InDirectBlock ind_block;
            d.readBlock(inode.pointer[11], &ind_block);

            int index = (number-10-N)/N;
            int rmd = (number-10-N) % N;
            if (ind_block.pointer[index] == 0) {
                return -1;
            }

            InDirectBlock ind_block2;
            d.readBlock(ind_block.pointer[index], &ind_block2);
            return ind_block2.pointer[rmd];

        } else if (number < 10 + N + N * N ) {
            printf ("Too big to support in current fs\n");
            return -1;
        } else {
            printf ("Too big to support in current fs\n");
            return -1;
        }
  }

  int FileSystem::write(int fd, char* buffer, int len) {
      if (!ft.isValid(fd)){
          return -1;
      }
      FileDescriptor & fdescr =  *ft.fdArray[fd];
      Inode & inode = fdescr.inode;

      int seekPtr = fdescr.seekPtr;
      int logic_block_nr = (fdescr.seekPtr)/BLOCKSIZE;
      int offset = fdescr.seekPtr % BLOCKSIZE;

      int left = len;
      int total_written = 0;

      char tmp_buffer[BLOCKSIZE];

      while( left > 0 ){
          int block_nr = 0;
          if (logic_block_nr >= (inode.fileSize + BLOCKSIZE -1 )/ BLOCKSIZE) {
              block_nr = insertBlock(inode, logic_block_nr);
          }
          else {
              block_nr = getBlockNumber(inode, logic_block_nr);
          }
          //cout << "block_nr = " << block_nr << std::endl;
          if (block_nr <= 0) return(-1);

          int write_size = BLOCKSIZE-offset < len ? BLOCKSIZE-offset : len;
          //cout << "write size = " << write_size << std::endl;
          d.readBlock(block_nr, &tmp_buffer);
          memcpy(tmp_buffer + offset, buffer+total_written, write_size);
          d.writeBlock(block_nr, &tmp_buffer);

          //cout << "left = " << left << std::endl;
          left -= write_size;
          total_written += write_size;
          offset = 0;
          logic_block_nr++;
          seekPtr += write_size;

          ft.setSeekPointer(fd, seekPtr);
          if (seekPtr > inode.fileSize)
              inode.fileSize = seekPtr;
      }
      return total_written;
  }

  int FileSystem::read(int fd, char* buffer, int len) {
       if (!ft.isValid(fd)){
           return -1;
       }
       FileDescriptor & fdescr =  *ft.fdArray[fd];
       Inode & inode = fdescr.inode;
       int seekPtr = fdescr.seekPtr;
       int logic_block_nr = (fdescr.seekPtr)/BLOCKSIZE;
       int offset = fdescr.seekPtr % BLOCKSIZE;

       int left = len;
       int total_read = 0;

       char tmp_buffer[BLOCKSIZE];

       while( left > 0 ){
           int block_nr = getBlockNumber(inode, logic_block_nr);
           if (block_nr <= 0) return(-1);

           int read_size = BLOCKSIZE-offset < len ? BLOCKSIZE-offset : len;
           d.readBlock(block_nr, &tmp_buffer);
           memcpy(buffer+total_read, tmp_buffer + offset, read_size);

           left -= read_size;
           total_read += read_size;
           offset = 0;
           logic_block_nr++;
           seekPtr += read_size;
           ft.setSeekPointer(fd, seekPtr);
       }
       return total_read;
   }

  int FileSystem::seek(int fd, int offset, int whence){
      int seekptr = ft.getSeekPointer(fd);
      int fileSize = ft.getInode(fd)->fileSize;
      switch(whence){
          case SEEK_SET:
              seekptr = offset;
              break;
          case SEEK_CUR:
              seekptr += offset;
              break;
          case SEEK_END:
              seekptr = fileSize + offset;
              break;
          default: ;
      }
      ft.setSeekPointer(fd, seekptr);
      return seekptr;
  }


  int FileSystem::close(int fd){
      Inode inode = ft.getInode(fd);
      char block[BLOCKSIZE];
      bool updated = false;
      for (int i=1; i < sb.iSize; i++) {
          d.readBlock(i, block);
          InodeBlock * ib = reinterpret_cast<InodeBlock*>(block);

          for (int j=0; j < INODE_PER_BLOCK; j++) {
              Inode inode2 = ib->node[j];
              if (inode2.flags !=0 && inode2.owner == inode.owner){
                  ib->node[j] = inode;
                  d.writeBlock(i, block);
                  updated = true;
                  break;
              }
          }
          if (updated)
              break;
      }
      if (!updated) {
          printf("Inode not found");
          return -1;
      }

      ft.free(fd);
      return 0;
  }
  int FileSystem::del (int inumber) {return 0;}
