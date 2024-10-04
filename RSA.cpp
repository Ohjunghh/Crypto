#include <iostream>
#include <gmpxx.h>
#include <ctime>

using namespace std;

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
mpz_class nextprime(mpz_class n){
    mpz_class r;
    mpz_nextprime(r.get_mpz_t(),n.get_mpz_t());
    return r;
}
mpz_class urandomb(gmp_randstate_t state,unsigned int bit){
    mpz_class r;
    mpz_urandomb(r.get_mpz_t(),state,bit);
    return r;
}
mpz_class sizeinbase(mpz_class op,int base){
    return mpz_sizeinbase(op.get_mpz_t(),base);
}
mpz_class random_prime(unsigned int bit){
    mpz_class r,p;
    unsigned long seed;
    gmp_randstate_t state;

    seed=time(NULL);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state,seed);

    r=urandomb(state,bit);
    p=nextprime(r);

    if(sizeinbase(p,2)!=bit)
        return random_prime(bit);
    else
        return p;
}
int cmp(mpz_class a,mpz_class b){
    int r;
    r=mpz_cmp(a.get_mpz_t(),b.get_mpz_t());
    return r;
}
mpz_class lcm(mpz_class a,mpz_class b){
    mpz_class r;
    mpz_lcm(r.get_mpz_t(),a.get_mpz_t(),b.get_mpz_t());
    return r;
}
mpz_class powm(mpz_class base,mpz_class exp,mpz_class mod){
    mpz_class r;
    mpz_powm(r.get_mpz_t(),base.get_mpz_t(),exp.get_mpz_t(),mod.get_mpz_t());
    return r;
}
mpz_class mod_inv(mpz_class x,mpz_class mod){
    mpz_class r;
    if(mpz_invert(r.get_mpz_t(),x.get_mpz_t(),mod.get_mpz_t())==0)
        cout<<"error : an inverse doesn't exist"<<endl;
    return r;
}
mpz_class gcd(mpz_class a,mpz_class b){
    mpz_class r;
    mpz_gcd(r.get_mpz_t(),a.get_mpz_t(),b.get_mpz_t());
    return r;
}

void RSA_KeyGen(struct RSA_PK *pk,struct RSA_SK *sk,int key_size){
    mpz_class ell;

    sk->p=random_prime(key_size/2);
    sk->q=random_prime(key_size/2);

    while(cmp(sk->p,sk->q)==0)
        sk->q=random_prime(key_size/2);
    sk->N=sk->p*sk->q;
    pk->N=sk->N;

    ell=lcm(sk->p -1,sk->q -1);

    pk->e=0x10001;

    if(gcd(pk->e,ell)!=1)
        pk->e=nextprime(pk->e);
    
    sk->e=pk->e;
    sk->d=mod_inv(pk->e,ell);
}
mpz_class RSA_encrypt(struct RSA_PK pk,mpz_class msg){
    mpz_class ct;
    ct=powm(msg,pk.e,pk.N);
    return ct;
}
mpz_class RSA_decrypt(struct RSA_SK sk,mpz_class ct){
    mpz_class msg;
    msg=powm(ct,sk.d,sk.N);
    return msg;
}

int main(){
    struct RSA_PK pk;
    struct RSA_SK sk;
    int key_size=2048;
    mpz_class msg=0x48656c6c6f21;
    mpz_class ct;
    mpz_class pt;

    RSA_KeyGen(&pk,&sk,key_size);
    cout << "msg : " << msg << endl << endl;

    ct=RSA_encrypt(pk,msg);
    cout << "Ciphertext : " <<  ct << endl << endl;

    pt=RSA_decrypt(sk,ct);
    cout << "Plaintext: " <<  pt << endl << endl;

    return 0;
}