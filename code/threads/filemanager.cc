
#include "synch.h"
#include "filemanager.h"
FileManager::FileManager(){
	for(int i=0;i<FILEMAXNUMBER;i++){
		semaphoreW[i]=new Semaphore("filesavers",1);
		semaphoreR[i]=new Semaphore("filesavers",1);
	}
	currIndex=0;
}
fileStatus
FileManager::ReadStatus(char *filename){
	return stats[getFileIndex(filename)];
}
void
FileManager::RequestReadAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->P();
}
void
FileManager::RequestWriteAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->P();
	semaphoreR[thisIndex]->P();
}
void
FileManager::ReleaseReadAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->V();
}
void
FileManager::ReleaseWriteAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->V();
	semaphoreR[thisIndex]->V();
}
void
FileManager::AddNewFile(char *filename){
	filetables[currIndex]=filename;
	stats[currIndex]=SPACING;
	printf("%s\n", filetables[currIndex]);
	currIndex++;
}
int
FileManager::getFileIndex(char *filename){

	for (int i = 0; i < currIndex+1; ++i)
	{
		/* code */
		if(filetables[i]!=NULL){
			if (strcmp(filetables[i],filename)==0)
			{
			/* code */
			return i;
			}
		}
	}
	return -1;
}
