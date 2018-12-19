# nachos With FS
Objective
In this project, we are going to implement Distributed File System on Nachos. So the whole project has two parts, basic file system and distributed file system. 
In part one, we finished following things. 
1)For synchronization, concurrently access/use the same file with multiple threads, and all file operations become atomic and serializable; moreover, if a file is in use, thread cannot remove it until all other threads close the file. 
2) We increase the maximum file size to 128KB.
3) We implement extensible file size. So when user write bytes more than the file size, the size of the file will be increased automatically.
4) We add system calls such as Create, Read, Write, Print to enable file operations in user programs.
In part two, we implements following things. 
1)Nachos system supports communicates using sockets
2)One Nachos system can send requests to Read/Write local or other Nachos systems’ files and can do correct reactions to the requests.
3)We implement system calls like “ReadTo” “WriteTo” “Listening” to enable target request in the user programs.


What has been developed
	Basic File System:

Increase the maximum file size. 
In original Nachos file system, all files’ header are stored in one sector, and it only use direct header to allocate sector. This results that there are only 30 direct headers, so the maximum file size can only be 30*128 bytes, which is less than 4000 bytes. The disk size of Nachos is 128KB, so our goal is to make the maximum file size to be 128KB. In our design, we choose to use doubly indirectly header. The doubly indirectly header will points to 32 indirect headers, and every indirect header will point to 32 sectors; thus the maximum size of a file becomes 32*32*128 bytes, which is 128KB.
	To achieve this design, we modify two files filehdr.h and filehdr.cc.

	filehdr.h
In this file we change the defined variables. We add “sectorPerIndirect” = 32, which means every indirect header points to 32 sectors. We add “sectorPerDoublyIndirect” = 32, which means doubly indirect header points to 32 indirect header. We add which means “NumDoublyIndirect” = 1,  there is only one doubly indirect header, and this is enough to reach the maximum file size of 128KB. We also change “NumDirect” to 29, originally there is 29 direct header, since we add one doubly indirect header, the direct header number becomes 29. As a result, the maximum file size now becomes (NumDirect*SectorSize + sectorPerIndirect*sectorPerDoublyIndirect*SectorSize) > 128KB. We also add two new class, DoubleIndirectHeader and IndirectHeader. Each of them has an size of 32 array (sectorPointer), and these array is used to store the sector number, so we can find the corresponding file/file header by them. Otherwise, they also have two functions, FetchFrom() and WriteBack(). They are same as the original function in the file header, we use them to write file header back or fetch file header from disk. Finally, we also add doublyIndirectSector in class FileHeader, this is used to store the sector number of doubly indirect header’s sector, so we can fetch it later quickly.

fileHdr.cc
In this class, we mainly modify three functions, Allocation(), ByteToSector(), and Deallocate(). We talk about Allocation() first. This function is called when file system creates a new file, so the file size and bitmap will be given. We use the file size to calculate how many sector the file need. Then if it only require less than 30 sector, we can allocate sector to it by direct sector (the allocation is done by allocate the slot of bitmap and write the corresponding sector number to the sector array of the header). Otherwise, we first allocate all direct sector to the file. Then we allocate a sector to the doubly indirect header. And then we use a nested for loop to allocate sector to the remaining required sectors. In the outer for loop, we create a indirect header, and every indirect header will allocate 32 sector to the file which is also the inner loop. We stop to allocate sector to the file if the required sector number of the file is fulfilled (we use variable allocatedSector to track the number) or there is no more sector can be allocated (this should not be happen because we will make sure there is enough sector before we allocate sector to the file). After we allocated sector for the file, we also write all used indirect header and doubly indirect header back to corresponding sector. So we can fetch them later, when we need them
ByteToSector(), an offset will be passed in to this function, and we use the offset to return the corresponding sector number. We calculate the sector number by offset/SectorSize, if the value is less than 30, we know it is in a direct sector, so we can return is quickly. Otherwise, we need to find the sector by search on doubly indirectly header. We can reach it by function FetchFrom(doublyIndirectSector), because it points to 32 indirect header, we can find and fetch the indirect header by FetchFrom(offset/SectorSize/sectorPerIndirect). Finally, because the indirect also points to 32 sectors, we can get the sector number by the indirect header sector array with index of (offset/SectorSize)%sectorPerIndirect. Then we return  the sector number.
Deallocate(), this is similar to the allocate. We first starts at direct sector, and we deallocate all its occupied sector by function Clear() of bitmap. Then if the file also allocated sector by doubly indirect header and indirect header, we also use nested for loop to fetch them and deallocate all of them, we still call function Clear() of bitmap to deallocate them; moreover we also need to set the corresponding sector array element of the indirect header and doubly indirect header to -1.

One more thing, because when we implement doubly indirect header and indirect header, we need to allocate sector for them (33 sectors, which is 4000+KB), so the maximum file size cannot be 128KB, the actually maximum size is around 124KB. 


Implement extensible file size
In original Nachos, when we create a file, we must give the file size; once the file is allocated with this size, we cannot change it anymore. So we want to change such situation. When we write to a file if the file size is not enough to hold the writing bytes, the system will call function extend() to allocate more sector to the file and increase the file size. We make change in filesys.cc, openfile.cc, and filehdr.cc.
filesys.cc
We add new function extend() in this class. And this function will be called in WriteAt() of openFlie.cc, when the size of writing bytes is larger than the file size. And then this file will call extendFileSize() which is a function of filehdr.cc. And sector bitmap and writing bytes must be pass to this file. So extendFileSize() can allocate more sector to these writing bytes.

openFile.cc
When we write to a file by calling function WriteAt(), we need to check if the writing bytes + writing position will exceed the file size or not; if yes, then we need to increase the file size by allocating more sector to the file. To allocate more sector, we need the file header and the sector bitmap. So we call extend() in filesys.cc, so we can get the file of sector bitmap. Then extend() will pass the bitmap and corresponding file header to the function extendFileSize() which is in filehdr.cc, so in extendFileSize() we have the file header and the bitmap, then we can allocate more sector to the fiel.

filehdr.cc
We add new function extendFileSize() to this class FileHeader. And the function takes two parameter, the sector bitmap and the writing bytes. We first check if there is enough sector for the new writing bytes; if yes, we check if the new writing bytes + original file size will exceed disk size. If no, then we allocate new sectors to the file. We first allocate all available direct sector to the file. If it is not enough, then we allocate sectors by doubly indirect header and indirect header. The allocation process is very similar to the process in function Allocation(), which we took before. The only difference is that, the doubly indirect header and indirect header may already allocated; so if they are allocated, we need to fetch them back from dick, and allocate sector with them if they still have empty slot in their sector array.

Moreover, because now we support extensible file size, we can increase the value of NumDirEntries, so the system can support more than 10 file in a directory. 


Synchronizations of Read/Write/Delete:
To achieve the synchronization control, we create a big table of all open-files to manage all the files. A file manager object is created to do these management. We set two semaphores for each open-file. One is a semaphore for the Read operation and one is for the Write operation. The main reason we need two seperate semaphores is that read and write should have different conflicts. Which means, when one Read is doing on a file, other Read operations are able to execute but Write operations should be blocked to wait for all readings on this file being down, while a Write operation can block other Write operations and all other Read operations.
It is a classic Reader-Writer problem. One approach to achieve it is that a Write operation P() on both Read semaphore and Write semaphore and V() when it is finished, and a Read operation only P() on read semaphore only when it is the first Read on the current resource and V() only when it is the last Read operation on the target resource that will finish.
filemanager.cc\filemanager.h
These two files define a global file table and two global semaphore tables  to do the synchronization. Once a file is created, the file name will be added to the file table and will gain two semaphores at the same index of those tables. 
One kernel should have one global filemanager.
For each write/read operations, we provide Request/Release operations to make safer write/read. The Requests operation will P() on the required resources and Release will V() them. For the delete, We simply consider it needs Semaphores for both Read and Write. After the delete operation finishes, the file name table removes the out-of-date file name and set the two semaphores to original version.
Filesys.cc
Once an open file is created, it writes to the file manager table.
kernel.h/kernel.cc
Create an object for file manager in kernel object and initialize it.


Distributed File System:

System calls
To implement DFS, we add some system calls to the system. They are WriteTo(), ReadTo(), Create(), Print(), Read(), Write(), Listening(), Exit().

WriteTo(), this function is used to write content to a file in remote server. It takes 4 input, addressString, which should be in the form of “node/file”, so the system can know which send to which server. And the file is to let the server know to open which file. Size1, the size of the addressString. Content, the content that the user want to write on the target file. Size2, the size of the content. Once the system call is triggered, the portion of code in SC_WriteTo in expceptionHandler() will be invoked, and it will read the addressString and content, and then it put them in different buffer, and function WriteTo() will process and send them out to target server. If the user input has no “addr/” in the beginning, then the system will try to open a locate file and write on it.

ReadTo(), this function is used to read content from a file in remote server. It also takes 4 input, the first two are addressString and size. The last two are a buffer and the buffer size. This buffer is used to store the read content. Once the system call is triggered in a user program, the portion of code in SC_ReadTo in expceptionHandler()  will be invoked, and it will read the addressString and content, and then it put them in different buffer, and function ReadFrom() will process and send them out to target server. If the user input has no “addr/” in the beginning, then the system will try to open a locate file and read from it.

Create(), this function is used to create a new file, two parameters must be given. First is the file name, second is the size of the name. Once the system call is triggered in a user program, the portion of code in SC_ReadTo in expceptionHandler() will be invoked, and it will call function filesys->Create() to create the file.

Print(), this function is used to print content of a buffer in the console. Buffer and the size of content needed to print must be given as parameters to the function.  Once the system call is triggered in a user program, the portion of code in SC_Print in expceptionHandler() will be invoked, and it will read the buffer content from main memory and print it to the console.

Read(), this function is used to read content of file which is in the local system. 4 parameters must be given. The file name, file name size, buffer to store read content, and the buffer size.  Once the system call is triggered in a user program, the portion of code in SC_Read in expceptionHandler() will be invoked, and it will open the file by the given file name, and it will read the content and write them in the buffer. 

Write(), this function is used to write content to a file which is in the local system. 4 parameters must be given. The file name, file name size, writing content, and content size.  Once the system call is triggered in a user program, the portion of code in SC_Write in expceptionHandler() will be invoked, and it open the file by file name, and then it will call function WriteAt() to write content to the opened file.

Listening(), this function is used to listen on port, so the current server can receive request from other machine. Once the system call is triggered in a user program, the portion of code in SC_Listening in expceptionHandler() will be invoked, and it will call function Listening() to listen on port.

Exit(), this function is used to finish a running thread. Once the system call is triggered in a user program, the portion of code in SC_Exit in expceptionHandler() will be invoked, and it will call Finish() to finish current thread.

Local and Remote Read/Write operations
For communicating between two Nachos process, we use original socket communication to send a Read/Write request to a server.
sender.cc/sender.h
These two files define two functions to do the communication: 
WriteTo and ReadFrom
Due to the System Calls used in a user program, the program transforms file name format like “1/File1” to a real read/write request and send to the right server. To distinguish each Nachos terminal, each Nachos should be created with its hostname set using “-m integer” parameter. The command “WriteTo(1/File1/Hello_World)” can be translated to a request to write on file which name is “File1” on node 1 with content Hello_world. Similarly, the command “ReadTo(1/File1)” can be translated to a request reading file ”File1“ on node 1. After each request is generated, the program send the requests to the correct nodes. One thing should be warned that this program uses 10 successive ports for node 0 to node 9, starts from the 9094.
Exception - Listening(int node)
In the exception.cc file, a server function is implemented. Once the user program calls “Listening()” system call, the node becomes a server receiving requests towards it. Using an infinite loop, the node server can provide file resources until we shut it down manually. 

For local file read and write, Write function reads a part of char array and write the content into the target file. Read function provides an empty char array and read the whole file to it.

 



How to test our solution?

Before testing:
Go to nachos/test directory, make
Go to nachos/code/build.linux make depend
Go to nachos/code/build.linux make

Testing maximum file size, we run following command: ./nachos -K 0
In this test case, the system will create a file with size 124000, then we write an input buffer with size 120000 to the file, and the we read content of the file and print it on the console.

Testing extensible file size, we run following command: ./nachos -K 1
In this test case, the system will create a file with size 0, then we write an input buffer with size 120000 to the file, so the system will extend the size of file and allocate more sectors to it. Then we read the content of the file and print it on the console.

Testing extensible file size, we run following command: ./nachos -K 7
In this test case, the system will create a file with size 0, then we create an input buffer with size 20000 to the file. We write the buffer into the files by three times. First time we write 1000 bytes. Second time we write 10000 bytes. Last time we write 9000 bytes. So every time we write to the file, the system will extend the size of the file.

Testing more than 10 file in a directory, we run following command:  ./nachos -K 2
In this test case, the system will create 12 files, each has size of 1000. 

After you test the basic file system, you should make clean to empty the whole disk.

For synchronizations testing, we should make Read/Write operations atomic. However, if we write the synchronization directly into the ReadAt/WriteAt function, it will become hard to test if it succeeds. In a word, if we want to test if the synchronization system is making effect, we should run a confliction adding it manually and run again without the system.
 ./nachos -K 4: Two writing operations are conflict without synchronization and seriable system.
 ./nachos -K 3: Two writing operations are conflict with synchronization and seriable system.
	 ./nachos -K 6: One delete operation and one write operation are conflict without synchronization and seriable system.
 ./nachos -K 5: One delete operation and one write operation are conflict without synchronization and seriable system.
Besides, the synchronization system has added to the distributed system and will make effects automatically. Details are in the “exception.cc”--Listening(int node) function. Semaphore request and release are with all the write/read file operations.

	Testing distributed system:
*Test1.c creates two files “File1” and “File2" and waits for other requests come. Lets call it node 0.
*Test2.c creates two files “File2_1” and “File2_2” and read the contents in the “File1” file in node 1 and print it. Then, it waits for other requests come. Let’s call it node 1.
*Test3.c overwrites the file “File2_1”from node 1  with its own char arrays and then straightly read from that file and print it.
In three different windows:(With the correct order)
./nachos -x “../test/test1” -m 0
./nachos -x “../test/test2” -m 1
./nachos -x “../test/test3” -m 2

Note: If the server can’t run with the error message “Address already used”, you should kill the process using that PORT.[currently it is 9094 - 9094+m, m is the numbers of the distributed nachos and has a maximum with 9]
