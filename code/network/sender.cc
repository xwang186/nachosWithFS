// Client side C/C++ program to demonstrate Socket programming 

#include "sender.h"
#define PORT 9094 
//测试成功~
int ParseAddress(char *address,char *filename){
    int nodeNumber=(int)address[0]-(int)'0';
    printf("(int)1-(int)0=%d\n", (int)'1'- (int)'0');
    if (nodeNumber<0 || nodeNumber>9)
    {
        /* code */
        nodeNumber=-1;
    }
    int size=strlen(address);
    for (int i = 0; i < size-2; ++i)
    {
        /* code */
        filename[i]=address[i+2];
    }
    filename[size-2]='\0';
    return nodeNumber;
}
int WriteTo(char *addressString, char *content){
    char *realFileName=new char[20];
    int numberOfNode=ParseAddress(addressString,realFileName);
    if (numberOfNode==-1)
    {
        /* code */
        printf("File name Format wrong!\n");
    }

    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT+numberOfNode); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    char *request=new char[100];
    request[0]='1';
    request[1]='|';
    int lenthOfFileName=strlen(realFileName);
    for(int i=0;i<lenthOfFileName;i++){
        request[i+2]=realFileName[i];
    }
    request[lenthOfFileName+2]='/';
    for (int i = 0; i < strlen(content); ++i)
    {
        /* code */
        request[lenthOfFileName+i+3]=content[i];
    }
    request[lenthOfFileName+strlen(content)+3]='\0';
    printf("Request: %s\n", request);
    send(sock , request , strlen(request) , 0 );
    printf("Write request message sent\n"); 
    char *response=new char[1000];
    valread = read( sock , response, 1024); 
    printf("%s\n",response ); 
    return 0; 
}
int ReadFrom(char *addressString, char *content){
    char *realFileName=new char[20];
    int numberOfNode=ParseAddress(addressString,realFileName);
    if (numberOfNode==-1)
    {
        /* code */
        printf("File name Format wrong!\n");
    }
        printf("numberOfNode is %d\n", numberOfNode);
        printf("Address String is %s\n", addressString);
    printf("Reading from port %d...\n", PORT+numberOfNode);
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT+numberOfNode); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    char *hello=new char[9];
    char *request=new char[100];
    request[0]='0';
    request[1]='|';
    for(int i=0;i<strlen(realFileName);i++){
        request[i+2]=realFileName[i];
    }
    request[strlen(realFileName)+2]='\0';
    //hello= "1/wwww11/I am the new content of www111"; 
    send(sock , request , strlen(request) , 0 );
    printf("Hello message sent\n"); 
    valread = read( sock , content, 1024); 
    printf("%s\n",content ); 
    return 0; 
}
char TrasferString(char *name, char *filename,char *content){
    int size = strlen(name);
    char type=name[0];
    if (type=='0')
    {
        /* code */
        for(int i=0; i<size-2; i++){
            filename[i] = name[i+2];
        }
        filename[size-2]='\0';
    }
    else if (type=='1')
    {
        /* code */
        int i=2;
        char curr=name[2];
        while(curr!='/'){
            filename[i-2]=name[i];
            i++;
            curr=name[i];
        }
        filename[i-2]='\0';
        int contentIndex=0;
        i++;
        while(i<size){
            content[contentIndex]=name[i];
            i++;
            contentIndex++;
        }
        content[contentIndex]='\0';
    }
    else{
        printf("Request format unexpected!\n");
    }
    return type;
}

