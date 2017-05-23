#include <iostream>
#include <string>
#include "bplus.h"

using namespace std;

int main()
{
    string path = "./data.txt";

    bPlusTree<int> bpt(path);

    bpt.print(bpt.root);

    bpt.insert(6,1006);
    bpt.print(bpt.root);





}
