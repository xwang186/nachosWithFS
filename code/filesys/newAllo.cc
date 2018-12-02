bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    cout<<numSectors<<"^^^^^^"<<endl;
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
            for(int k=0; k<IndirectSize && processedSector<numSectors; k++){
                indirectSector[k] = freeMap->FindAndSet();
                processedSector++;
            }
            //cout<<processedSector<<" "<<numSectors<<"******"<<endl;
            kernel->synchDisk->WriteSector(sectorForindirectIndex, (char *)indirectSector);
        }
    }
    cout<<processedSector<<"&&&&&&"<<endl;
    

    return TRUE;
}