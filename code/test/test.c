#include "syscall.h"

int
main()
{
  
   //char *name1 = "";

   //char *name2 = "File2_2";
    char *content="I am node 3，I will change all of you!!!!";
    char buf[50];
  //Write("test", 4, "abcdefghijklmnopqrstuvwxyz", 26);
  //Read("test", 4, buf, 1000);

 // Print(buf, 1000);
  WriteTo("1/File2_1", 10, content, 50);
   ReadTo("1/File2_1",10,buf,50);
   Print(buf, 50);
    
  Exit(0);
}