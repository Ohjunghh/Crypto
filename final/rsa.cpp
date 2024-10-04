#include <iostream>
#include <gmpxx.h>

using namespace std;

//RSA e N d N p q
struct RSA_PK{
    mpz_class e;
    mpz_class N;
};
struct RSA_SK{
    mpz_class e;
    mpz_class d;
    mpz_class N;
    mpz_class p;
    mpz_class q;
};
int cmp(mpz_class  a,mpz_class b){
    return mpz_cmp(a.get_mpz_t(),b.get_mpz_t());
}
mpz_class gcd(mpz_class a,mpz_class  b){
    mpz_class r;
    mpz_gcd(r.get_mpz_t(),a.get_mpz_t(),b.get_mpz_t());
    return r;
}
mpz_class lcm(mpz_class  a,mpz_class  b){
    mpz_class r;
    mpz_lcm(r.get_mpz_t(),a.get_mpz_t(),b.get_mpz_t());
    return r;
}
mpz_class inv_mod(mpz_class base,mpz_class mod){
    mpz_class r;
    if(mpz_invert(r.get_mpz_t(),base.get_mpz_t(),mod.get_mpz_t())==0)
        cout<<"no inverse"<<endl;
    return r;
}
mpz_class mod(mpz_class base,mpz_class mod){
    mpz_class r;
    mpz_mod(r.get_mpz_t(),base.get_mpz_t(),mod.get_mpz_t());
    return r;
}
mpz_class powm(mpz_class base,mpz_class exp,mpz_class mod){
    mpz_class r;
    mpz_powm(r.get_mpz_t(),base.get_mpz_t(),exp.get_mpz_t(),mod.get_mpz_t());
    return r;
}
mpz_class urandomb(gmp_randstate_t state,unsigned int bit){
    mpz_class r;
    mpz_urandomb(r.get_mpz_t(),state,bit);
    return r;
}
mpz_class nextprime(mpz_class a){
    mpz_class r;
    mpz_nextprime(r.get_mpz_t(),a.get_mpz_t());
    return r;
}
mpz_class random_prime(unsigned int bit){
    mpz_class r,p;
    gmp_randstate_t state;
    unsigned long seed;

    seed=time(NULL);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state,seed);

    r=urandomb(state,bit);
    p=nextprime(r);

    if(mpz_sizeinbase(p.get_mpz_t(),2)!=bit)
        return random_prime(bit);
    else
        return p;

}
void RSA_keygen(RSA_PK *pk,RSA_SK *sk,int key_size){
    mpz_class ell;
    sk->p=random_prime(key_size/2);
    sk->q=random_prime(key_size/2);

    while(cmp(sk->p,sk->q)==0)
        sk->q=random_prime(key_size/2);
    
    sk->N= sk->p*sk->q;
    pk->N=sk->N;

    ell=lcm(sk->p-1,sk->q-1);
    pk->e=0x10001;

    if(gcd(pk->e,ell)!=1)
        pk->e=nextprime(pk->e);
    sk->e=pk->e;

    sk->d=inv_mod(sk->e,ell);


}
mpz_class RSA_encrypt(RSA_PK pk,mpz_class msg){
    mpz_class ct;
    ct=powm(msg,pk.e,pk.N);
    return ct;
}
mpz_class RSA_decrypt(RSA_SK sk,mpz_class ct){
   mpz_class msg;
   msg=powm(ct,sk.d,sk.N);
   return msg;
}
mpz_class RSA_sign(RSA_SK sk,mpz_class msg){
    mpz_class sig;
    sig=powm(msg,sk.d,sk.N);
    return sig;
}
bool RSA_verify(RSA_PK pk,mpz_class sig,mpz_class msg){
    mpz_class r;
    r=powm(sig,pk.e,pk.N);
    if(cmp(r,msg)==0)
        return true;
    else
        return false;
}
mpz_class RSA_decrypt_CRT(RSA_SK sk,mpz_class ct){
   mpz_class m,m1,m2;
   mpz_class inv_M1,inv_M2;

   m1=powm(ct,sk.d,sk.p);
   m2=powm(ct,sk.d,sk.q);

   inv_M1=inv_mod(sk.q,sk.p);
   inv_M2=inv_mod(sk.p,sk.q);

   m=mod(m1*sk.q*inv_M1,sk.N);
   m+=mod(m2*sk.p*inv_M2,sk.N);
   m=mod(m,sk.N);

   return m;
}
int main(){
    RSA_PK pk;
    RSA_SK sk;
    mpz_class ct;
    mpz_class dmsg;
    mpz_class sig;
    mpz_class msg=0x111111;
    mpz_class  msg2=0x333333;
    bool rop1,rop2;

    RSA_keygen(&pk,&sk,2048);

    sig=RSA_sign(sk,msg);
    ct=RSA_encrypt(pk,msg);
    dmsg=RSA_decrypt_CRT(sk,ct);
    //dmsg=RSA_decrypt(sk,ct);
        cout<<"0x"<<hex<<dmsg;
    cout<<endl;
    rop1=RSA_verify(pk,sig,dmsg);
    rop2=RSA_verify(pk,sig,msg2);

    if(rop1==true)
        cout<<"true"<<endl;
    else
        cout<<"false"<<endl;
    
    if(rop2==true)
        cout<<"true"<<endl;
    else
        cout<<"false"<<endl;

    return 0;
}