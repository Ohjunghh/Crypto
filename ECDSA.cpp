#include<iostream>
#include<gmpxx.h>
#include<random>

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
mpz_class urandomm(gmp_randstate_t state,const mpz_class n){
    mpz_class rop;
    mpz_urandomm(rop.get_mpz_t(),state,n.get_mpz_t());
    return rop;
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

void ECDSA_keygen(ECDSA_PK *pk,ECDSA_SK *sk){
    
    unsigned long seed;
    gmp_randstate_t state;
    random_device rd;

    pk->E={2,2,17};
    sk->E=pk->E;

    pk->G={5,1};
    sk->G=pk->G;

    pk->n=19;
    sk->n=pk->n;
    
    seed=rd();
    gmp_randinit_mt(state);
    gmp_randseed_ui(state,seed);

    sk->d=urandomm(state,sk->n);
    
    EC_mult(sk->E,&sk->Q,sk->G,sk->d);

    pk->Q=sk->Q;

}

int main(){

    return 0;
}