#include <iostream>
#include <string>
#include <map>
using namespace std;

#ifdef _WIN32
typedef long long my_off_t;
#else
typedef long my_off_t;
#endif

map<string, string> m;

extern "C" void init() {
    m.clear();
}

extern "C" void insert(char*a, char *s) {
    string p=a;
   
    m[p] = s;
}

extern "C" void del(char*s) {
   string p=s;
    if (!m.count(p)) {
        terminate();
    }
    m.erase(p);
}

extern "C" int myget(char*a,char*s) {
   string p=s;
    if (!m.count(p)) {
        terminate();
    }
    if(!(m[p]==a)){
          printf("*read error\n");
         cout<<"expect value: "<<m[p]<<"       \n";
    }
    return m[p]==a;
}


extern "C" size_t get_total() {
    return m.size();
}
extern "C" size_t get_key(int k,char*s){
    auto it = m.begin();
    for (int i = 0; i < k; ++i) {
      
        ++it;
    }
   string p = it->first;
    for(int i=0;i<p.length();i++)
    *(s+i)=p[i];
    *(s+p.length())='\0';
    return p.length();
}
