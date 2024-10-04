#include <gmpxx.h>
#include "aes.h"
#define BLOCK_SIZE 16

bool AES_GCM_decrypt(unsigned char *pmsg,unsigned char *tag,unsigned char *ct,int len,unsigned char *iv,unsigned char *key,unsigned char *aad);

template<class It>void int2bnd(int n,It begin, It end)
{
    for(It i=end; i!=begin;n/=0x100)
        *--i=mpz_class{n % 0x100}.get_ui();
}

void AES_CTR_encrypt(unsigned char *ct,unsigned char *pmsg,int len, unsigned char *iv,unsigned char *key)
{
    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);

    for(int i=0;i<len;i+=16)
    {
        int2bnd(i/16+1,ivNctr+12,ivNctr+16);
        AES_encrypt(ivNctr+i,key,ct+i);
    }
    for(int i=0;i<len;i++)
    {
        ct[i]^=pmsg[i];
    }
}

void AES_CTR_decrypt(unsigned char *pmsg,unsigned char *ct,int len,unsigned char *iv, unsigned char *key)
{
    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);

    for(int i=0;i<len;i+=16)
    {
        int2bnd(i/16+1,ivNctr+12,ivNctr+16);
        AES_encrypt(ivNctr+i,key,pmsg+i);
    }
    for(int i=0;i<len;i++)
    {
        pmsg[i]^=ct[i];
    }
}

void H_doub(unsigned char *p)
{
    bool bit1=p[15]&1;
    for(int i=15;i>0;i--)
        p[i]=(p[i]>>1)|(p[i-1]<<7);
    p[0]>>=1;
    if(bit1)
        p[0]^=0b11100001;
}

void GF_mult(unsigned char *x,unsigned char *y)
{
    unsigned char z[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};
    for(int i=0;i<16;i++)
    {
        for(int j=0;j<8;j++)
        {  
            if(y[i]&1<<(7-j))
            {
                for(int k=0;k<16;k++)
                    z[k]^=x[k];
            }
            H_doub(x);
        }
    }
    memcpy(x,z,16);
}

void GHASH(unsigned char *h,unsigned char *ct,int len, unsigned char *aad, unsigned char *key)
{
    unsigned char H[16];
    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};

    memcpy(h,aad,16);
    AES_encrypt(msg,key,H);
    GF_mult(h,H);

    for(int i=0;i<len;i+=16)
    {
        for(int j=0;j<16;j++)
            h[j]^=ct[i+j];
        GF_mult(h,H);
    }
}

void AES_GCM_encrypt(unsigned char *tag,unsigned char *ct,unsigned char *pmsg,int len,unsigned char *iv,unsigned char *key,unsigned char *aad)
{
    AES_CTR_encrypt(ct,pmsg,len,iv,key);
    GHASH(tag,ct,len,aad,key);

    unsigned char lenAC[16];
    int2bnd(16,lenAC,lenAC+8);
    int2bnd(len,lenAC+8,lenAC+16);
    for(int i=0;i<16;i++)
        tag[i]^=lenAC[i];

    unsigned char H[16];
    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};
    AES_encrypt(msg,key,H);
    GF_mult(tag,H);

    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);
    int2bnd(0,ivNctr+12,ivNctr+16);
    AES_encrypt(ivNctr,key,ivNctr);
    for(int i=0;i<16;i++)
        tag[i]^=ivNctr[i];
}

bool AES_GCM_decrypt(unsigned char *dmsg,unsigned char *tag,unsigned char *ct,int len,unsigned char *iv,unsigned char *key,unsigned char *aad)
{
    AES_CTR_decrypt(dmsg,ct,len,iv,key);

    unsigned char h[16];
    GHASH(h,ct,len,aad,key);

    unsigned char lenAC[16];
    int2bnd(16,lenAC,lenAC+8);
    int2bnd(len,lenAC+8,lenAC+16);
    for(int i=0;i<16;i++)
        h[i]^=lenAC[i];

    unsigned char H[16];
    unsigned char msg[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};
    AES_encrypt(msg,key,H);
    GF_mult(h,H);

    unsigned char ivNctr[16];
    memcpy(ivNctr,iv,12);
    int2bnd(0,ivNctr+12,ivNctr+16);
    AES_encrypt(ivNctr,key,ivNctr);
    for(int i=0;i<16;i++)
        h[i]^=ivNctr[i];

    int j=0;
    for(int i=0;i<16;i++)
        if(tag[i]==h[i])
            j++;

    if(j==16)
        return true;
    else
        return false;
}


int main()
{
    unsigned char msg[16]="Hello world";
    unsigned char aes_128_key[]     ={0x98,0xff,0xf6,0x7e,0x64,0xe4,0x6b,0xe5,
                                    0xee,0x2e,0x05,0xcc,0x9a,0xf6,0xd0,0x12};
    unsigned char iv[]              ={0x2d,0xfb,0x42,0x9a,0x48,0x69,0x7c,0x34,
                                    0x00,0x6d,0xa8,0x86};
    unsigned char aad[16]={0xa0,0xca,0x58,0x61,0xc0,0x22,0x6c,0x5b,
                            0x5a,0x65,0x14,0xc8,0x2b,0x77,0x81,0x5a};

    int len=((sizeof(msg)/BLOCK_SIZE)+1)*BLOCK_SIZE;
    unsigned char pmsg[len];
    unsigned char ct[len];
    unsigned char tag[len];
    unsigned char dmsg[len];

    bool rop;

    int pad=len-sizeof(msg);

    for(int i=0; i<sizeof(msg);i++)
        pmsg[i]=msg[i];
    for(int i=sizeof(msg);i<len;i++)
        pmsg[i]=pad;

    for(int i=0;i<16;i++)
        printf("0x%02X ",msg[i]);
    printf("\n");

    AES_GCM_encrypt(tag,ct,pmsg,len,iv,aes_128_key,aad);

    for(int i=0;i<16;i++)
        printf("0x%02X ",ct[i]);
    printf("\n");
    
    for(int i=0;i<16;i++)
        printf("0x%02X ",tag[i]);
    printf("\n");

    rop=AES_GCM_decrypt(dmsg,tag,ct,len,iv,aes_128_key,aad);
    for(int i=0;i<16;i++)
        printf("0x%02X ",dmsg[i]);
    printf("\n");

    if(rop==true)
        cout<<"success"<<endl;
    else if(rop==false)
        cout<<"fail"<<endl;
        
    return 0;
}