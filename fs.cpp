//
//  fs.cpp
//  SimpleFS
//
//  Created by Jian Wu on 5/25/18.
//  Copyright © 2018 JackWu. All rights reserved.
//

#include "fs.hpp"
#include “disk.h"
"
FileSystem::formatDisk {
    d.openDisk("DISK", FS_SIZE * BLOCKSIZEß);
    d.zeroDisk();
    
    sb.size = FS_SIZE;
    sb.iSize = INODE_SIZE;
    
    for(int i; i < INODE_NUM_BLOCKS; i++) {
        InodeBlock ib;
        
    }
    
}







void main(int argc, char**argv) {
    
    
}
