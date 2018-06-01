/*
 * fs_test.cpp
 *
 *  Created on: May 30, 2018
 *      Author: Jack Wu
 */
#include "fs.h"

int main(int argc, char**argv) {

    FileSystem fs;
    cout << "argc " << argc << std::endl;
    if (argc == 2)
        fs.formatDisk(FS_NUM_BLOCKS, INODE_NUM_BLOCKS);

    int fd = fs.create(5);                  // create file with filename 5
    std::cout << "Creating file:" << std::endl;
    std::cout << "File Table: " << fs.ft.toString() << std::endl;
    std::cout << "fd = " << fd << std::endl;

    char buffer[20480] = "abcde";            // test buffer for writing
    for (int i=0; i<20480; i++) {
        //buffer[i] = "a";
    }
    if (fd == -1)
        fd = fs.open(5);
    fs.write(fd, buffer, 5);                   // write to file

    int fd2 = fs.create(6);
    if (fd2 == -1)
        fd2 = fs.open(6);

    fs.seek(fd2, 0, SEEK_END);
    std::cout << "After seek: File Table: " << fs.ft.toString() << std::endl;

    fs.write(fd2, buffer, 20480);

    std::cout << "Writing to file:\n";
    std::cout << fs.ft.toString() << std::endl;


    fs.close(fd);                           // close file

    std::cout << "CLosing to file:\n";


    fd = fs.open(5);                        // open file

    std::cout << "\nOpening file:\n";
    std::cout << fs.ft.toString();
    std::cout << std::endl;
    char* buffer2 = new char[5];           // test buffer for reading
    fs.read(fd, buffer2, 5);                   // read file

    std::cout << "Dumping the file content:\n";
    for (int i=0; i<5; i++)    // dump the read buffer
    {
        std::cout << buffer2[i] << std::endl;
    }

    std::cout << fs.ft.toString();
    fs.close(fd);                           // close file
    fs.close(fd2);

    return 0;
}



