#include "aes.h"
#include <gmpxx.h>

template<class IT>void int2bnd(int n,IT b,IT e){
    for(IT i=e;i!=b;n/=0x100)
        *--i=mpz_class{n%0x100}.get_ui();
}

void AES_CTR_encrypt(unsigned char *ct,unsigned char *pmsg,int len,unsigned char *iv,unsigned char *key){
    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);

    for(int i=0;i<len;i+=BLOCK_SIZE){
        int2bnd(i/BLOCK_SIZE+1,ivNctr+12,ivNctr+16);
        AES_encrypt(ivNctr,key,ct+i);
    }
    for(int i=0;i<len;i++)
        ct[i]^=pmsg[i];
}
void AES_CTR_decrypt(unsigned char *pmsg,unsigned char *ct,int len,unsigned char *iv,unsigned char *key){
    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);

    for(int i=0;i<len;i+=BLOCK_SIZE){
        int2bnd(i/BLOCK_SIZE+1,ivNctr+12,ivNctr+16);
        AES_encrypt(ivNctr,key,pmsg+i);
    }
    for(int i=0;i<len;i++)
        pmsg[i]^=ct[i];
}
void H_doub(unsigned char *p){
    bool bit=p[15]&1;
    for(int i=15;i>0;i--){
        p[i]=(p[i]>>1)|(p[i-1]<<7);
    }
    p[0]>>=1;
    if(bit)
        p[0]^=0b11100001;
    
}
void GF_mult(unsigned char *x,unsigned char *y){
    unsigned char z[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    for(int i=0;i<BLOCK_SIZE;i++){
        for(int j=0;j<8;j++){
            if(y[i]&(1<<(7-j))){
                for(int k=0;k<16;k++)
                    z[k]^=x[k];
            }
             H_doub(x);   
        }
    }
    memcpy(x,z,16);
}
void GHASH(unsigned char *h,unsigned char *ct,int len,unsigned char *aad,unsigned char *key){
    memcpy(h,aad,16);
    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char H[16];
    AES_encrypt(msg,key,H);
    GF_mult(h,H);

    for(int i=0;i<len;i+=BLOCK_SIZE){
        for(int j=0;j<BLOCK_SIZE;j++){
            h[j]^=ct[i+j];
        }
        GF_mult(h,H);
    }

}
void AES_GCM_encrypt(unsigned char *tag,unsigned char *ct,unsigned char *pmsg,int len,unsigned char *iv,unsigned char *key,unsigned char *aad){
    AES_CTR_encrypt(ct,pmsg,len,iv,key);
    GHASH(tag,ct,len,aad,key);

    unsigned char lenAC[16];
    int2bnd(16,lenAC,lenAC+8);
    int2bnd(len,lenAC+8,lenAC+16);
    for(int i=0;i<BLOCK_SIZE;i++)
        tag[i]^=lenAC[i];

    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char H[16];
    AES_encrypt(msg,key,H);
    GF_mult(tag,H);

     unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);
    int2bnd(0,ivNctr+12,ivNctr+16);
    AES_encrypt(ivNctr,key,ivNctr);
    for(int i=0;i<BLOCK_SIZE;i++)
        tag[i]^=ivNctr[i];

}

bool AES_GCM_decrypt(unsigned char *pmsg,unsigned char *tag,unsigned char *ct,int len,unsigned char *iv,unsigned char *key,unsigned char *aad){
    AES_CTR_decrypt(pmsg,ct,len,iv,key);
    unsigned char h[16];
    GHASH(h,ct,len,aad,key);

    unsigned char lenAC[16];
    int2bnd(16,lenAC,lenAC+8);
    int2bnd(len,lenAC+8,lenAC+16);
    for(int i=0;i<BLOCK_SIZE;i++)
        h[i]^=lenAC[i];

    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char H[16];
    AES_encrypt(msg,key,H);
    GF_mult(h,H);

    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);
    int2bnd(0,ivNctr+12,ivNctr+16);
    AES_encrypt(ivNctr,key,ivNctr);
    for(int i=0;i<BLOCK_SIZE;i++)
        h[i]^=ivNctr[i];

    for(int i=0;i<16;i++){
        if(tag[i]!=h[i])
            return false;
    }
    return true;
}

int main(){
    
    unsigned char msg[]="Hello world";
    unsigned  char aes_128_key[]={0x98,0xff,0xf6,0x7e,0x64,0xe4,0x6b,0xe5,0xee,0x2e,0x05,0xcc,0x9a,0xf6,0xd0,0x12};
    unsigned char iv[]={0x2d,0xfb,0x42,0x9a,0x48,0x69,0x7c,0x34,0x00,0x6d,0xa8,0x86};
    unsigned char aad[16]={0xa0,0xca,0x58,0x61,0xc0,0x22,0x6c,0x5b,0x5a,0x65,0x14,0xc8,0x2b,0x77,0x81,0x5a};
   
    int len=(sizeof(msg)/BLOCK_SIZE+1)*BLOCK_SIZE;
    unsigned char tag[16],ct[len],pmsg[len],dmsg[len];
    bool rop;
    int pad=0;//len-sizeof(msg);
    memcpy(pmsg,msg,sizeof(msg));
    for(int i=sizeof(msg);i<len;i++)
        pmsg[i]=pad;

    for(int i=0;i<sizeof(msg);i++)
        printf("0x%02X ",msg[i]);
    printf("\n");
    for(int i=0;i<len;i++)
        printf("0x%02X ",pmsg[i]);
    printf("\n");
    AES_GCM_encrypt(tag,ct,pmsg,len,iv,aes_128_key,aad);
    for(int i=0;i<len;i++)
        printf("0x%02X ",ct[i]);
    printf("\n");
    for(int i=0;i<16;i++)
        printf("0x%02X ",tag[i]);
    printf("\n");
    rop=AES_GCM_decrypt(dmsg,tag,ct,len,iv,aes_128_key,aad);
    for(int i=0;i<len;i++)
        printf("0x%02X ",dmsg[i]);
    printf("\n");
    for(int i=0;i<sizeof(msg);i++)
        printf("0x%02X ",dmsg[i]);
    printf("\n");


    if(rop==true)
        printf("success!");
    else
        printf("fail!");
    return 0;
  

}