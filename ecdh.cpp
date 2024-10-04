#include <iostream>
#include <vector>

using namespace std;


struct Point{
    int x;
    int y;
};

struct EC{
    int a;
    int b;
    int p;
};

int mod_inv(int x,int mod){
    int r;

    while(x<0)
        x+=mod;
    
    if(x==0)
        return 0;
    
    else if(mod<=0)
        return -1;
    
    else{
        for(r=0;r<mod;r++){
            if(r*x%mod==1)
                return r;
        }
    return -1;
    }
}

void EC_add(EC E,Point *R,Point P,Point Q){
    int r;

    if(P.x==0&&P.y==0)
    {
        *R=Q;
    }
    else if(Q.x==0&&Q.y==0)
    {
        *R=P;
    }
    else{
        r=mod_inv(Q.x-P.x,E.p);

        if(r==0){
            R->x=0;
            R->y=0;
        }
        else if(r<0){
            cout << "error" << endl;
        }
        else{
            r*=(Q.y-P.y);
            r%=E.p;

            R->x=r*r-P.x-Q.x;
            while(R->x<0){
                R->x+=E.p;
            }
            R->x%=E.p;

            R->y=r*(P.x-R->x)-P.y;
            while(R->y<0){
                R->y+=E.p;
            }
            R->y%=E.p;
        }
    }
}

void EC_double(EC E,Point *R,Point P){
    int r;
    r=mod_inv(2*P.y,E.p);

    if(r==0){
        R->x=0;
        R->y=0;
    }
    else if(r<0){
        cout << "error" << endl;
    }
    else{
        r*=(P.x*P.x)*3+E.a;
        r%=E.p;

        R->x=r*r-2*P.x;
        while(R->x<0){
            R->x+=E.p;
        }
        R->x%=E.p;

        R->y=r*(P.x-R->x)-P.y;
        while(R->y<0){
            R->y+=E.p;
        }
        R->y%=E.p;
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

        for(;n-2>=0;n--){
            EC_double(E,R,*R);
            if(bits.at(n-2)==1)
                EC_add(E,R,*R,P);
        }
    }
}

int main(){

    Point A,B;
    EC ec;
    Point R;

    A={1,1};

    ec.a=2;    ec.b=3;    ec.p=5;

    EC_mult(ec,&R,A,7);
    
    cout<< "7A = ("<<R.x<<","<<R.y<<")"<<endl;

    return 0;
}