// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    order = kernel->orderNum++;
    //cout<<order<<"+++"<<endl;
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    //cout<<numSectors<<"^^^^^^"<<endl;
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    
    int processedSector = 0;
    if(numSectors < NumDirect){
        for (int i = 0; i < numSectors; i++) {
        dataSectors[i] = freeMap->FindAndSet();
        // since we checked that there was enough free space,
        // we expect this to succeed
        ASSERT(dataSectors[i] >= 0);
        }
    }
    else {
        //int processedSector = 0;
        for (int i = 0; i < NumDirect; i++) {
        dataSectors[i] = freeMap->FindAndSet();
        // since we checked that there was enough free space,
        // we expect this to succeed
        ASSERT(dataSectors[i] >= 0);
        processedSector = i;
        }
        //cout<<processedSector<<"&&&&&&"<<endl;
        for(int j=0; j<NumIndirect && processedSector<numSectors; j++){
            int indirectSector[IndirectSize];
            int sectorForindirectIndex = freeMap->FindAndSet();
            // cout<<"--------------------"<<endl;
            //cout<<"allocated sector "<<sectorForindirectIndex<<" at position "<<numOfIndirectSector<<endl;
            for(int k=0; k<IndirectSize && processedSector<numSectors; k++){
                indirectSector[k] = freeMap->FindAndSet();
                //cout<<indirectSector[k]<<" get sector"<<endl;
                processedSector++;
            }
            //cout<<"--------------------"<<endl;
            //cout<<processedSector<<" "<<numSectors<<"******"<<endl;
            kernel->synchDisk->WriteSector(sectorForindirectIndex, (char *)indirectSector);
            kernel->fileEntry[order].UsedIndirectSector[++numOfIndirectSector] = sectorForindirectIndex;
            //numOfIndirectSector++;

        }
    }
    //cout<<processedSector<<"&&&&&&"<<endl;
    

    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
    	//ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    	freeMap->Clear((int) dataSectors[i]);
    }

    for(int i=0; i<1; i++){
        char *temp = new char[SectorSize];
        kernel->synchDisk->ReadSector(kernel->fileEntry[order].UsedIndirectSector[i], temp);
        int *sector = (int *)temp;
        for(int j=0; j<IndirectSize; j++){
            freeMap->Clear((int) sector[j]);
        }
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    //return(dataSectors[offset / SectorSize]);
    int sectorOffset = offset / SectorSize;
    //cout<<sectorOffset<<"!!!!!"<<endl;
    //cout<<order<<"+++"<<numOfIndirectSector<<endl;
    if(sectorOffset < NumDirect) return(dataSectors[sectorOffset]); 
    else{
        int remainingSectors = sectorOffset - NumDirect;
        // cout<<leftSectors<<"*****"<<endl;
        int indirectIndex = (remainingSectors-1)/32;

        char *temp = new char[SectorSize];
        //cout<<numOfIndirectSector<<"hahahahaha"<<endl;
        cout<<kernel->fileEntry[order].UsedIndirectSector[indirectIndex]<<" at position "<<indirectIndex<<endl;
        kernel->synchDisk->ReadSector(kernel->fileEntry[order].UsedIndirectSector[indirectIndex], temp);
        int *sector = (int *)temp;
        int indexOffset = sectorOffset - NumDirect - indirectIndex*32 - 1;
        
        //cout<<sector[indexOffset]<<"~~~~~~"<<endl;
        return sector[indexOffset];
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}


bool FileHeader::extendFileSize(PersistentBitmap *freeMap, int fileSize){

    int extraSectors = divRoundUp(fileSize, SectorSize);
    int originalSize = FileLength();
    int originalSector = divRoundUp(originalSize, SectorSize);
    if(order == -1) order = kernel->orderNum++;
    //cout<<"hahahaha"<<endl;

    //there is no enough free sector
    if (freeMap->NumClear() < extraSectors)
        return FALSE;       // not enough space

    //there is no enough sector
    if(originalSector + extraSectors > NumDirect+NumIndirect*IndirectSize) 
        return FALSE;

    int processedSector = 0;
    for(int i=numSectors; i<NumDirect; i++){
        //if(dataSectors[i] == -1) {
            dataSectors[i] = freeMap->FindAndSet();
            processedSector++;
        //}
    }

    for(int j=0; j<NumIndirect && processedSector<extraSectors; j++){
        int indirectSector[IndirectSize];
        int sectorForindirectIndex = freeMap->FindAndSet();
        // cout<<"--------------------"<<endl;
        // cout<<"allocated sector "<<sectorForindirectIndex<<" at position "<<numOfIndirectSector<<endl;
        for(int k=0; k<IndirectSize && processedSector<extraSectors; k++){
            indirectSector[k] = freeMap->FindAndSet();
            // cout<<indirectSector[k]<<" get sector"<<endl;
            processedSector++;
        }
        // cout<<"--------------------"<<endl;
        //cout<<processedSector<<" "<<numSectors<<"******"<<endl;
        kernel->synchDisk->WriteSector(sectorForindirectIndex, (char *)indirectSector);
        kernel->fileEntry[order].UsedIndirectSector[numOfIndirectSector] = sectorForindirectIndex;
        numOfIndirectSector++;
    }

    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	kernel->synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}
