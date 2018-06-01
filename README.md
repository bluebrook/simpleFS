# simpleFS

A hobby c++ projects to implement simple fs based on a JAVA course projects
* projects requirements: http://codex.cs.yale.edu/avi/os-book/OS9/java-dir/12.pdf
* source code [http://www.cs.odu.edu/~cs471w/code/oscj8e-final-src/ch11/File%20system%20project/]

## data structures
The low level disk operation to read/write blocks are simulated using a file in unix-like system. The key data structures involved 
* SuperBlock: stored at the begining of the disk, records the total number of blocks, num of inode blocks and first free block
* InodeBlock: blocks to hold pointer to Inode
* InDirectBlock: blocks to hold pointer to other blocks
* Inode: keep filesize, blocks
* FileDescriptor: fd and inode
* FileTable: a table of int fd and FileDescriptor objects

## Make
make(needs to have c++11 gcc)
./test
make clean
