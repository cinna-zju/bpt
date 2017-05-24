#include <iostream>
#include <string>
#include "bplus.h"

using namespace std;

int main()
{
    string path = "./data.txt";
    int v[7] = {1,3,2,5,4,7,6};
    int p[7] = {1001,1003,1002,1005,1004,1007,1006};
    bPlusTree<int> bpt(path);

    // for(int i = 0; i < 5; i++){
    //     bpt.insert(v[i], p[i]);
    // }
    //
    // bpt.insert(7,1007);
    //
    for(int i = 1; i <= 22; i++){
        bpt.insert(i, 1000+i);
    }

    bpt.print(bpt.root);
    //bpt.printLeaf();


}
