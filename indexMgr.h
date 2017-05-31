#include <string>
#include <cstdio>
#include <iostream>
#include "bplus.h"

using namespace std;

class indexMgr{
public:
    int create(string, string, string);
    int drop(string);
    int search(string, string);
    int insert(string, string, int);
    int deleteByKey(string, string);
}

int indexMgr::create(string indexName, string tableName, string attribute, int type)
{
    cout << "creating index on" + attribute << endl;

    string filename = "./index/"+indexName;


    if(type == TYPE_INT){
        bPlusTree<int> bpt(filename);
        bpt.init();
    }

    if(type == TYPE_FLOAT){
        bPlusTree<float> bpt(filename);
        bpt.init();
    }

    if(type == TYPE_STRING){
        bPlusTree<string> bpt(filename);
        bpt.init();
    }

    cout << "created succesfully" << endl;
    return 0;

}

int indexMgr::drop(string indexName)
{
    string file = "./index/"+"indexName";
    if(remove(file) == 0){
        return 0;
    }else{
        return errno;
    }
}

template <typename T>
int indexMgr::search(string filename, T val)
{
    bPlusTree<T> r(filename);
    offset result = r.search(val);
    return result;
}

template <typename T>
int indexMgr::deleteByKey(string filename, T val)
{
    bPlusTree<T> r(filename);
    offset result = r.search(val);
    if(result == -1){
        cout << "not find" << endl;
        return -1;
    }else{
        r.deleteValue(val);
        return 0;
    }
}

template <typename T>
int indexMgr::insert(string filename, T val, offset addr)
{
    bPlusTree<T> r(filename);
    r.insert(val, addr);
    return 0;
}
