#include<iostream>
#include<vector>

using namespace std;

bool is_primitive(int base,int mod){
    int exp=2;
    int r=base*base;
    for(;r!=base;exp++){
        r*=base;
        r%=mod;
    }
    return exp==mod;
}

vector<int> primitive_root(int mod){ //mod의 모든 원시근 찾아줌
    vector<int> v;
    for(int i=2;i<mod;i++){
        if(is_primitive(i,mod)==true)
            v.push_back(i);
    }
    return v;
}

int main(){
    for(int i : primitive_root(29))
        cout << i << ',';
    cout << endl;
}