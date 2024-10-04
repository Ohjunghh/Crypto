#include <iostream>
#include <string>

using namespace std;

string caesar_encode(string message,int key){
    for(char &ch : message) ch+=key;
    return message;
}
string caesar_decode(string message,int key){
    for(char &ch : message) ch-=key;
    return message;
}

int main(){
    string m="Caesar's cipher test";
    cout << (m=caesar_encode(m,3)) << endl;
    cout << caesar_decode(m,3) << endl;
}