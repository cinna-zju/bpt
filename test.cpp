#include <iostream>
#include <string>
#include "bplus.h"

using namespace std;

int main()
{
    string path = "./data.txt";

    bPlusTree<int> bpt(path);
    bpt.init();
    //bpt.getRoot();
    //bpt.print(bpt.root);

    //
    for(int i = 1; i <= 24; i++){
        bpt.insert(i, 1000+i);
    }
    //bpt.print(bpt.root);
    //
    // for(int i = 1; i <= 24; i++){
    //      bpt.insert(i, 1000+i);
    // }
    //
    //bpt.print(bpt.root);
    // bpt.printLeaf();

    //cout << bpt.search(3)<<endl;
    // cout << bpt.search(24)<<endl;
    // cout << bpt.search(1)<<endl;
    // cout << bpt.search(16)<<endl;
    // cout << bpt.search(0)<<endl;
    // cout << bpt.search(160)<<endl;
    cout<<"after delete---------"<<endl;

    bpt.deleteValue(9);
    cout << bpt.search(3)<<endl;

    //bpt.getRoot();
    //bpt.print(bpt.root);
    bpt.printLeaf();

}
