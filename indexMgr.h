#include <string>
#include <iostream>

using namespace std;

class indexMgr{
public:
    int create(string, string, string);
    int drop(string);
    int search(string, string);
    int insert(string, string, int);
    int deleteByKey(string, string);
}

int indexMgr::create(string indexName, string tableName, string attribute)
{
    

}
