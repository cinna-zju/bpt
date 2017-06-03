#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include<string>
using namespace std;



class Attribute{
public:
	string stringName;
	string type;
	int length;
	bool ifUnique;
	Attribute(string s, string t, bool i, int l){
		stringName = s;
		type = t;
		ifUnique = i;
		length = l;
	}
};

#endif
