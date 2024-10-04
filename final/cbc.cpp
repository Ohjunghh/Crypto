#include "aes.h"

void AES_CBC_encrypt(unsigned char *ct,unsigned char *pmsg,int len,unsigned char *iv,unsigned char *key){
    memcpy(ct,pmsg,len);

    for(int i=0;i<BLOCK_SIZE;i++)
        ct[i]^=iv[i];
    
    for(int i=0;i<len/BLOCK_SIZE-1;i++){
        AES_encrypt(ct,key,ct);
        for(int j=0;j<BLOCK_SIZE;j++,ct++)
            *(ct+BLOCK_SIZE)^=*ct;
    }
    AES_encrypt(ct,key,ct);
}

void AES_CBC_decrypt(unsigned char *dmsg,unsigned char *ct,int len,unsigned char *iv,unsigned char *key){
    memcpy(dmsg,ct,len);

    for(int i=0;i<len;i+=BLOCK_SIZE)
        AES_decrypt(dmsg+i,key,dmsg+i);

    for(int i=0;i<BLOCK_SIZE;i++,dmsg++)
        *dmsg^=iv[i];
    for(int i=0;i<len-BLOCK_SIZE;i++,dmsg++)
        *dmsg^=ct[i];
}
   

int main(){
    //gcc cbc.cpp -lstdc++ -lgmpxx -lgmp -o cbc
    unsigned char msg[]="Single block msg";
    unsigned char aes_128_key[] = {0x06,0xa9,0x21,0x40,0x36,0xb8,0xa1,0x5b,0x51,0x2e,0x03,0xd5,0x34,0x12,0x00,0x06};
    unsigned char iv[] = {0x3d,0xaf,0xba,0x42,0x9d,0x9e,0xb4,0x30,0xb4,0x22,0xda,0x80,0x2c,0x9f,0xac,0x41};

    /*int len,pad,num;
    len=(sizeof(msg)/BLOCK_SIZE+1)*BLOCK_SIZE;
    pad=len-sizeof(msg);

    unsigned char pmsg[len],dmsg[len],ct[len];

    memcpy(pmsg,msg,sizeof(msg));
    for(int i=sizeof(msg);i<len;i++)
        pmsg[i]=pad;
    num=pmsg[len-1];
    for(int i=0;i<sizeof(msg);i++)
        printf("0x%02X ",msg[i]);
    printf("\n");
    */

   int len,num,pad;
   len=(sizeof(msg)/BLOCK_SIZE+1)*BLOCK_SIZE;
    pad=len-sizeof(msg);
    unsigned char pmsg[len],dmsg[len],ct[len];

    memcpy(pmsg,msg,sizeof(msg));
    for(int i=sizeof(msg);i<len;i++)
        pmsg[i]=pad;

    for(int i=0;i<len;i++)
        printf("0x%02X ",pmsg[i]);
    printf("\n");

    AES_CBC_encrypt(ct,pmsg,len,iv,aes_128_key);
    for(int i=0;i<len;i++)
        printf("0x%02X ",ct[i]);
    printf("\n");

    AES_CBC_decrypt(dmsg,ct,len,iv,aes_128_key);
    for(int i=0;i<len;i++)
        printf("0x%02X ",dmsg[i]);
    printf("\n");
    num=pmsg[len-1];
    for(int i=0;i<len-num;i++)
        printf("0x%02X ",dmsg[i]);
    printf("\n");


}