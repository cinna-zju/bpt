#include <iostream>
#include <string>
#include "bplus.h"

using namespace std;

int main()
{
    string path = "./data.txt";

    bPlusTree<int> bpt(path);


    bpt.read(bpt.root);

    //bpt.insert(1, 1000);
    bpt.insert(3, 1001);


    bpt.print(bpt.root);


    //cerr << bpt.root<<' '<< bpt.Bptfile <<endl;

    // int data = 123;
    // offset key = 2321;
    //
    // bpt.insertIntoTree(bpt.root, 0, 2321);
    // bpt.insertIntoTree(bpt.root, 1, 2325);


}
