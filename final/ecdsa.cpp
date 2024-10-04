#include <iostream>
#include <gmpxx.h>
#include <random>
#include <vector>

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
mpz_class urandomm(gmp_randstate_t state,mpz_class a){
    mpz_class r;
    mpz_urandomm(r.get_mpz_t(),state,a.get_mpz_t());
    return r;
}
mpz_class nextprime(mpz_class a){
    mpz_class r;
    mpz_nextprime(r.get_mpz_t(),a.get_mpz_t());
    return r;
}
void EC_double(EC E,Point *R,Point P){
    if(P.x==0&&P.y==0){
        R->x=0;
        R->y=0;
    }
    else{
        mpz_class r,xtemp,ytemp;
        mpz_class Rx,Ry;
        ytemp=inv_mod(2*P.y,E.p);
        if(ytemp==0){
            R->x=0;
            R->y=0;
        }
        else{
            xtemp=mod(3*P.x*P.x+E.a,E.p);
            r=mod(xtemp*ytemp,E.p);

            Rx=mod(r*r-2*P.x,E.p);
            R->x=Rx;

            Ry=mod(r*(P.x-Rx)-P.y,E.p);
            R->y=Ry;
        }
    }
}
void EC_add(EC E,Point *R,Point P,Point Q){
    if(P.x==0&&P.y==0){
        *R=Q;
    }
    else if(Q.x==0&&Q.y==0){
        *R=P;
    }
    else if(P.x==Q.x&&P.y==Q.y){
        EC_double(E,R,P);
    }
    else{
        mpz_class r,xtemp,ytemp;
        mpz_class Rx,Ry;

        xtemp=inv_mod(Q.x-P.x,E.p);
        if(xtemp==0){
            (*R).x=0;
            R->y=0;
        }
        else{
            ytemp=mod(Q.y-P.y,E.p);
        r=mod(xtemp*ytemp,E.p);

        Rx=mod(r*r-P.x-Q.x,E.p);
        R->x=Rx;

        Ry=mod(r*(P.x-Rx)-P.y,E.p);
        R->y=Ry;
        }
    }
}
void inv_bit(vector<bool> *bit,mpz_class r){
    for(;r>0;r/=2)
        (*bit).push_back(r%2==1);
}
void EC_mult(EC E,Point *R,Point P,mpz_class r){
   vector<bool> bit;
   int n;

    inv_bit(&bit,r);
    n=bit.size();

    *R=P;
    for(;n>1;n--){
        EC_double(E,R,*R);
        if(bit.at(n-2)) //????????????
            EC_add(E,R,*R,P);
    }
}

void ECDSA_keygen(ECDSA_PK *pk,ECDSA_SK *sk){
    pk->E={2,2,17};
    sk->E=pk->E;

    pk->G={5,1};
    sk->G=pk->G;

    pk->n=19;
    sk->n=pk->n;

    gmp_randstate_t state;
    random_device rd;
    unsigned long seed;

    do{
    gmp_randinit_mt(state);
    seed=rd();
    gmp_randseed_ui(state,seed);

    sk->d=urandomm(state,sk->n);
    }while(sk->d==0);

    EC_mult(sk->E,&sk->Q,sk->G,sk->d);
    pk->Q=sk->Q;

}
ECDSA_SIG ECDSA_sign(ECDSA_SK sk,mpz_class msg){
    mpz_class k;
    gmp_randstate_t state;
    random_device rd;
    unsigned long seed;
    Point R;
    ECDSA_SIG sig;
    int mBit,nBit;
    mpz_class h_m;

    mBit=mpz_sizeinbase(msg.get_mpz_t(),2);
    nBit=mpz_sizeinbase((sk.n).get_mpz_t(),2);
    h_m=msg>>max(mBit-nBit,0);

    do{
        do{
            gmp_randinit_mt(state);
            seed=rd();
            gmp_randseed_ui(state,seed);
            k=urandomm(state,sk.n);
        }while(k==0);
        
        EC_mult(sk.E,&R,sk.G,k);
    }while(mod(R.x,sk.n)==0);

    sig.S1=mod(R.x,sk.n);
    sig.S2=inv_mod(k,sk.n);
    sig.S2*=(h_m+mod(sig.S1*sk.d,sk.n));
    sig.S2=mod(sig.S2,sk.n);

    if(mod(sig.S2,sk.n)==0||inv_mod(sig.S2,sk.n)==0)
        return ECDSA_sign(sk,msg);
    else
        return sig;

}
bool ECDSA_verify(ECDSA_PK pk,ECDSA_SIG sig,mpz_class msg){
     mpz_class c;
      mpz_class u1,u2;
      Point R1,R2,R;

    int mBit,nBit;
    mpz_class h_m;

    mBit=mpz_sizeinbase(msg.get_mpz_t(),2);
    nBit=mpz_sizeinbase((pk.n).get_mpz_t(),2);
    h_m=msg>>max(mBit-nBit,0);

    c=inv_mod(sig.S2,pk.n);
    u1=mod(h_m*c,pk.n);
    u2=mod(sig.S1*c,pk.n);

    EC_mult(pk.E,&R1,pk.G,u1);
    EC_mult(pk.E,&R2,pk.Q,u2);
    EC_add(pk.E,&R,R1,R2);

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
    ECDSA_SIG sig1;
    mpz_class msg,not_msg;
    bool result;

    msg=0x111111111111;
    not_msg=0x333333333333;

    ECDSA_keygen(&pk,&sk);
    cout << "msg : " << msg << endl << endl;

    sig1=ECDSA_sign(sk,msg);
    cout << "S1 : " <<  sig1.S1 <<", "<< "S2 : " <<  sig1.S2<<endl << endl;

    result=ECDSA_verify(pk,sig1,msg);
    cout << "msg_verify_result: " <<  (result == true ? "true":"false") << endl << endl;

    result=ECDSA_verify(pk,sig1,not_msg);
    cout << "not_msg_verify_result: " <<  (result == true ? "true":"false") << endl << endl;


  
    return 0;
}