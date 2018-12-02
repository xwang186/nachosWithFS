#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "filemanager.h"

void
SimpleThread(int which)
{
    int num;

    printf("Open file correctly!\n");
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        kernel->currentThread->Yield();
    }
}
void
FileTest(int which){
	char* t="wwww";
    char* u="wwww";

    kernel->fileSystem->Create(t,30000);
    OpenFile *of=kernel->fileSystem->Open((char *)u);
   	char *input;
   	if (which==1)
   	{
   		input="Bye!";
   	}
   	else{
   		input="Hel!";
   	//input[1]=(char)(which+48);
   	}
   	kernel->filemanager->RequestWriteAt(t);
   	for (int i = 0; i < 10; ++i)
   	{
   		printf("Written once for process %d\n", which);

   		 of->WriteAt(input, 4, i*4);

   		 kernel->currentThread->Yield();
   	}
   	kernel->filemanager->ReleaseWriteAt(t);
    char* temp = new char[200];

    	of->ReadAt(temp, 100, 0);
        printf("%d: \n%s\n\n",which, temp);
        //kernel->currentThread->Yield();

}

void
ThreadTest()
{


    //kernel->fileSystem->Print();
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) FileTest, (void *) 1);
    
    FileTest(0);

}
