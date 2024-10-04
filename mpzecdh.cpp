#include<iostream>
#include<gmpxx.h>

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

void bits_inv(vector<int> *bits,int r){
    for(;r>0;r/=2)
        (*bits).push_back(r%2);
}

void EC_mult(EC E,Point *R,Point P,int r){
    vector<int> bits;
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


int main(){
    /*Point A,B,C;
    Point R;
    EC secp256k1; 

    secp256k1 = {0, 7, mpz_class{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"}};

    A={mpz_class{"0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"},
        mpz_class{"0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"}};
    
    B={mpz_class{"0xC6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5"},
        mpz_class{"0x1AE168FEA63DC339A3C58419466CEAEEF7F632653266D0E1236431A950CFE52A"}};

    C={mpz_class{"0xF9308A019258C31049344F85F89D5229B531C845836F99B08601F113BCE036F9"},
        mpz_class{"0x388F7B0F632DE8140FE337E62A37F3566500A99934C2231B6CB9FD7584B8E672"}};

    //EC_add(secp256k1,&R,A,B);
    EC_mult(secp256k1,&R,A,2);
    if(cmp(R.x,C.x)==0&&cmp(R.y,C.y)==0)
        cout << "success" <<endl;
    else
         cout << "fail" <<endl;*/
    
    EC secp256k1;
    //mpz_class dA,dB;
    /*int dA,dB;
    mpz_class G;
    Point QA,QB;
    Point ShareA,ShareB;

    secp256k1 = {0, 7, mpz_class{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"}};
    G{"79BE667E F9DCBBAC 55A06295 CE870B07 029BFCDB 2DCE28D9 59F2815B 16F81798 483ADA77 26A3C465 5DA4FBFC 0E1108A8 FD17B448 A6855419 9C47D08F FB10D4B8"};
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    //dA=urandomm(state,secp256k1.p);
    dA=3;
    EC_mult(secp256k1,&QA,G,dA);
    
    gmp_randinit_mt(state);
    //dB=urandomm(state,secp256k1.p);
    dB=2;
    EC_mult(secp256k1,&QB,G,dA);

    EC_mult(secp256k1,&ShareA,QB,dA);
    EC_mult(secp256k1,&ShareB,QA,dB);

    if(cmp(ShareA.x,ShareB.x)==0&&cmp(ShareA.y,ShareB.y)==0)
        cout << "success" <<endl;
    else
         cout << "fail" <<endl;
*/
    return 0;
}