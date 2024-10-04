#include <iostream>
#include <cstring>
#define BLOCK_SIZE 16;
using namespace std;

int main(){
    unsigned char msg[]="Hello";

    int len,pad,num;

    len=((sizeof(msg)/16)+1)*BLOCK_SIZE;
    pad=len-sizeof(msg);
    unsigned char pmsg[len];
    
    memcpy(pmsg,msg,sizeof(msg));
    for(int i=sizeof(msg);i<len;i++)
        pmsg[i]=pad;

    num=pmsg[len-1];
    for(int i=0;i<len-num;i++)
        printf("0x%02X ",pmsg[i]);
    
    cout<<endl;
    
    return 0;
}
