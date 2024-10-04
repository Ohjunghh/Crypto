#include <iostream>
#include <gmpxx.h>
#include <vector>
#include <ctime>
#include <random>
using namespace std;

struct Point{
    mpz_class x;
    mpz_class y;
};
struct EC{
    mpz_class a;
    mpz_class b;
    mpz_class p;
};
struct ECDSA_PK{
  EC E;
  Point G;
  mpz_class n;
  Point Q;  
};
struct ECDSA_SK{
    EC E;
    Point G;
    mpz_class n;
    Point Q;
    mpz_class d;
};
struct ECDSA_SIG{
  mpz_class S1;
  mpz_class S2; 
};
mpz_class urandomm(gmp_randstate_t state,mpz_class n){
    mpz_class r;
    mpz_urandomm(r.get_mpz_t(),state,n.get_mpz_t());
    return r;
}
mpz_class mod_inv(mpz_class x,mpz_class mod){
    mpz_class rop;
    if(mpz_invert(rop.get_mpz_t(),x.get_mpz_t(),mod.get_mpz_t())==0)
        cout<<"error : an inverse doesn't exist"<<endl;
    return rop;
}
mpz_class mod(mpz_class n,mpz_class p){
    mpz_class r;
    mpz_mod(r.get_mpz_t(),n.get_mpz_t(),p.get_mpz_t());
    return r;
}
int cmp(mpz_class x,mpz_class y){
    int ans;
    ans=mpz_cmp(x.get_mpz_t(),y.get_mpz_t());
    return ans;
}
void EC_double(EC E,Point *R,Point P){
    mpz_class r;
    mpz_class xtemp,ytemp;
    mpz_class Rx,Ry;

    ytemp=mod(2*P.y,E.p);
    ytemp=mod_inv(ytemp,E.p);

    if(ytemp==0){
        R->x=0;
        R->y=0;
    }
    else if(ytemp<0)
        cout<<"error"<<endl;
    else{
        xtemp=(3*P.x*P.x+E.a);
        xtemp=mod(xtemp,E.p);

        r=xtemp*ytemp;
        r=mod(r,E.p);

        Rx=r*r-2*P.x;
        Rx=mod(Rx,E.p);
        R->x=Rx;

        Ry=r*(P.x-Rx)-P.y;
        Ry=mod(Ry,E.p);
        R->y=Ry;
    }
}

void EC_add(EC E,Point *R,Point P,Point Q){
    mpz_class r;
    mpz_class xtemp,ytemp;
    mpz_class Rx,Ry;

    if(P.x==0&&P.y==0)
    {
        *R=Q;
    }
    else if(Q.x==0&&Q.y==0)
    {
        *R=P;
    }
    else if(P.x==Q.x&&P.y==Q.y){
        EC_double(E,R,P);
    }
    else{
        xtemp=Q.x-P.x;
        xtemp=mod(xtemp,E.p);
        xtemp=mod_inv(xtemp,E.p);

        if(xtemp==0){
            R->x=0;
            R->y=0;
        }
        else if(xtemp<0)
            cout<<"error"<<endl;
        else{
            ytemp=Q.y-P.y;
            ytemp=mod(ytemp,E.p);

            r=ytemp*xtemp;
            r=mod(r,E.p);

            Rx=r*r-P.x-Q.x;
            Rx=mod(Rx,E.p);
            R->x=Rx;

            Ry=r*(P.x-Rx)-P.y;
            Ry=mod(Ry,E.p);
            R->y=Ry;
        }
    }      
}
void bits_inv(vector<bool> *bits,mpz_class r){
    for(;r>0;r/=2)
        (*bits).push_back(r%2==1);
}
void EC_mult(EC E,Point *R,Point P,mpz_class r){
    vector<bool> bits;
    int n;

    if(P.x==0&&P.y==0){
        R->x=0;
        R->y=0;
    }
    else if(r<=0)
        cout<<"error"<<endl;
    else{
        bits_inv(&bits,r);
        n=bits.size();

        *R=P;

        for(;n>1;n--){
            EC_double(E,R,*R);
            if(bits.at(n-2)==1)
                EC_add(E,R,*R,P);
        }
    }
}

void ECDSA_KeyGen(struct ECDSA_PK *pk,struct ECDSA_SK *sk){
    unsigned long seed;
    gmp_randstate_t state;
    
    gmp_randinit_mt(state);
    seed=time(NULL);
    gmp_randseed_ui(state,seed);

    pk->E={2,2,17};
    sk->E=pk->E;

    pk->G={5,1};
    sk->G=pk->G;

    pk->n=19;
    sk->n=pk->n;

    sk->d=urandomm(state,sk->n);
    if(sk->d==0)
        ECDSA_KeyGen(pk,sk);
    else{
        EC_mult(sk->E,&sk->Q,sk->G,sk->d);
        pk->Q=sk->Q;
    }
}
ECDSA_SIG ECDSA_sign(struct ECDSA_SK sk,mpz_class msg){
    unsigned long seed;
    random_device rd;
    gmp_randstate_t state;
    ECDSA_SIG sig;
    mpz_class k,inv_k;
    Point R;
    mpz_class h_m;
    int nBit,mBit;

    nBit=mpz_sizeinbase(sk.n.get_mpz_t(),2);
    mBit=mpz_sizeinbase(msg.get_mpz_t(),2);
    h_m=msg >> max(mBit-nBit,0);

    gmp_randinit_mt(state);
    do{
        //seed=time(NULL);
        seed=rd();
        gmp_randseed_ui(state,seed);
        k=urandomm(state,sk.n);
        EC_mult(sk.E,&R,sk.G,k);
    }while(mod(R.x,sk.n)==0);

    sig.S1=mod(R.x,sk.n);

    inv_k=mod_inv(k,sk.n);
    if(inv_k==0)
        ECDSA_sign(sk,msg);

    sig.S2=inv_k*mod(h_m+sig.S1*sk.d,sk.n);
    sig.S2=mod(sig.S2,sk.n);

    if(mod(sig.S2,sk.n)==0||mod_inv(sig.S2,sk.n)==0)
        return ECDSA_sign(sk,msg);
    else
        return sig;
}


bool ECDSA_verify(struct ECDSA_PK pk,struct ECDSA_SIG sig,mpz_class msg){
    mpz_class c;
    mpz_class u1,u2;
    Point A,B,R;
    mpz_class h_m;
    int nBit,mBit;

    nBit=mpz_sizeinbase((pk.n).get_mpz_t(),2);
    mBit=mpz_sizeinbase(msg.get_mpz_t(),2);
    h_m= msg >> max(mBit-nBit,0); 

    c=mod_inv(sig.S2,pk.n);
    u1=mod(h_m*c,pk.n);
    u2=mod(sig.S1*c,pk.n);

    EC_mult(pk.E,&A,pk.G,u1);
    EC_mult(pk.E,&B,pk.Q,u2);
    EC_add(pk.E,&R,A,B);

    if(R.x==0&&R.y==0)
        return false;
    else{
        if(cmp(sig.S1,mod(R.x,pk.n))==0)
            return true;
    else
        return false;
    }
}

int main(){
    ECDSA_PK pk;
    ECDSA_SK sk;
    ECDSA_SIG sig;
    mpz_class msg,not_msg;
    bool result1;
    bool result2;

    msg=0x11111111111111;
    not_msg=0x12312312312312;

    ECDSA_KeyGen(&pk,&sk);
    cout << "msg : " << msg << endl << endl;

    sig=ECDSA_sign(sk,msg);
    cout << "S1 : " <<  sig.S1 <<", "<< "S2 : " <<  sig.S2<<endl << endl;

    result1=ECDSA_verify(pk,sig,msg);
    cout << "input_msg : " <<  msg ;
    
    cout << "     msg_verify_result : ";
    if(result1==true)
        cout<<"true"<<endl<<endl;
    else if(result1==false)
        cout<<"false"<<endl<<endl;
    else
        cout<<"error"<<endl<<endl;


    result2=ECDSA_verify(pk,sig,not_msg);
    cout << "input_msg : " <<  not_msg ;
    cout << "     msg_verify_result : ";
    
    if(result2==true)
        cout<<"true"<<endl<<endl;
    else if(result2==false)
        cout<<"false"<<endl<<endl;
    else
        cout<<"error"<<endl<<endl;

    return 0;
}