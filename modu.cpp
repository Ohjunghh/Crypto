#include<iostream>

using namespace std;

int modular(int base,int exp,int mod){
    int r=1;
    for(int i=0;i<exp;i++)
        r=(r*base)%mod;
    return r;
}

int main(){
    int base; printf("base : "); scanf("%d",&base);
    int exp; printf("exp : "); scanf("%d",&exp);
    int mod; printf("mod : "); scanf("%d",&mod);

    int ans=modular(base,exp,mod);
    cout << ans << endl;
}