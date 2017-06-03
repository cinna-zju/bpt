#ifndef BPLUS_H
#define BPLUS_H

#include <vector>
#include <iostream>
#include <cstdio>
#include <assert.h>

#include "Attribute.h"

//#define size 5

typedef long offset;


template <typename T>
class Node{
public:
    int num;// num of key
    int size;
    T *key;// array of key value
    offset *ptr;// array of ptr
    bool isLeaf;// whether this node is a leaf node

    Node(long size);


    long write(offset addr, FILE* fp);
    // write node data to file at addr
    // return num of bytes writed

    long read(offset addr, FILE* fp);
    // read node data from file at addr
    // return num of bytes read

    void print();
    // print node data
    // the first line is key from 0, num-1
    // the second line is ptr from 0, num
};


template <typename T>
class bPlusTree{
public:
    string fileName;
    offset root;// offset of root, addr: 0
    bool isUnique;// addr: 4
    bool isInit;// addr: 5
    long size;// fanout, addr: 9
    FILE *Bptfile;// fp

    bPlusTree(string filename, Attribute &a);
    // constructed function
    // if bptfile is exists
    // read root, size, isUnique to memory
    // else
    // create a bpt and initilize a root node

    ~bPlusTree();
    // save root, size, isUnique to file


    void insert(T val, offset addr);
    // (val, addr) is a record
    // insert one record into bpt

    void deleteValue(T val);
    // delete one (val, addr) pair in bpt
    // TODO: multiple value?

    void print(offset);
    // print the tree
    // only for height = 3
    // print tree of other height will crash

    void printLeaf();
    // print all the leaf node

    vector<offset> search(T val);
    // search val in bpt
    // return a vector containing the offset of result

private:
    //find the end of file and return offset
    inline offset getNewNode(){
        fseek(Bptfile, 0, SEEK_END);
        return ftell(Bptfile);
    };

    int insert(T, offset, offset);
    // insert in node at offset
    // (value, address), node offset
    // return 0 if corrected
    // return -1 if error occured

    void deleteValue(T, offset);
    // delete record in a node
    // used by deleteValue(T)

    void split(Node<T>&, Node<T>&, int);

    offset searchInNode(T);
    // only search in one node
    // used by deleteValue

};


template <typename T>
Node<T>::Node(long size):size(size)
{
    //when constructed, malloc memory

    key = (T*)malloc(sizeof(T) * size);
    ptr = (offset*)malloc(sizeof(offset) * (size+1));

}

template <typename T>
long Node<T>::write(offset addr, FILE *fp)
{
    fseek(fp, addr, SEEK_SET);
    fwrite(&num, sizeof(int), 1, fp);
    fwrite(&size, sizeof(int), 1, fp);
    fwrite(&isLeaf, sizeof(bool), 1, fp);
    fwrite(key, sizeof(T), size, fp);
    fwrite(ptr, sizeof(offset), size+1, fp);
    return ftell(fp) - addr;
}

template <typename T>
long Node<T>::read(offset addr, FILE *fp)
{
    fseek(fp, addr, SEEK_SET);
    fread(&num, sizeof(int), 1, fp);
    fread(&size, sizeof(int), 1, fp);
    fread(&isLeaf, sizeof(bool), 1, fp);
    fread(key, sizeof(T), size, fp);
    fread(ptr, sizeof(offset), size+1, fp);
    return ftell(fp) - addr;
}

template <typename T>
void Node<T>::print()
{
    //cout<<size<<" "<<num<<endl;
    for(int i = 0; i < num; i++){
        cout<<key[i]<<' ';
    }
    cout<<endl;

    for(int i = 0; i < num+1; i++){
        cout<<ptr[i]<<' ';
    }
    cout<<endl;
}

template <typename T>
bPlusTree<T>::bPlusTree(string filename, Attribute &a)
{
    Bptfile = fopen(filename.c_str(), "rb+");
    std::cout << Bptfile <<std::endl;

    if (Bptfile){
        // this index already exists

        cout << "init..." << endl;
        fseek(Bptfile, 0, SEEK_SET);
        fread(&isInit, sizeof(bool), 1, Bptfile);
        cout << "init?" << isInit <<endl;
        if(isInit == 1){
            fread(&root, sizeof(offset), 1, Bptfile);
            fread(&size, sizeof(int), 1, Bptfile);
            fread(&isUnique, sizeof(bool), 1, Bptfile);
        }else{
            // index file does not exists
            isUnique = a.ifUnique;
            isInit = 0;

            // size = (4096 - 2 * sizeof(int) - sizeof(bool)
            //     - sizeof(offset))/(a.length + sizeof(offset));
            size = 5;

            cout <<"size:"<<size<<endl;

            root = getNewNode()+4096;

            //cout<<"root addr:"<<root<<endl;
            Node<T> r(size);
            r.ptr[size] = 0;
            r.num = 0;
            r.isLeaf = true;
            r.write(root, Bptfile);
            isInit = 1;
        }
    }
}

template <typename T>
bPlusTree<T>::~bPlusTree()
{

    fseek(Bptfile, 0, SEEK_SET);
    fwrite(&isInit, sizeof(bool), 1, Bptfile);
    fwrite(&root, sizeof(offset), 1, Bptfile);
    fwrite(&size, sizeof(int), 1, Bptfile);
    fwrite(&isUnique, sizeof(bool), 1, Bptfile);
    fclose(Bptfile);
}

template <typename T>
void bPlusTree<T>::deleteValue(T val)
{
    deleteValue(val, root);

    Node<T> r(size);
    r.read(root, Bptfile);

    if(!r.isLeaf && r.num == 0){
        root = r.ptr[0];
    }
}

template <typename T>
void bPlusTree<T>::insert(T val, offset addr){

    Node<T> ro(size);
    ro.read(root, Bptfile);
    //if root node is full, split root node
    if(ro.num == size){
        //init a new root
        Node<T> nro(size);
        nro.num = 0;
        nro.isLeaf = false;
        nro.ptr[0] = root;//point to old root

        split(nro, ro, 0);//split(parent, current, idx)
        ro.write(root, Bptfile);//save the old root

        root = getNewNode();
        nro.write(root, Bptfile);//save new root
    }
    insert(val, addr, root);//after split if needed, insert into node


}

template <typename T>
int bPlusTree<T>::insert(T val, offset addr, offset cur){

    int idx;
    Node<T> r(size);
    r.read(cur, Bptfile);

    //find idx first large than val,  the position to insert the value
    for(idx = 0; idx < r.num && r.key[idx] < val; idx++)
        ;

    //key should be distinct
    // if(idx < r.num && r.isLeaf && r.key[idx] == val){
    //     cout << "duplicate key: " << val <<endl;
    //     return;
    // }

    //r is non-leaf node
    if( !r.isLeaf ){
        Node<T> s(size);//the node to insert into
        s.read(r.ptr[idx], Bptfile);

        if(s.num == size){
            //if s is full, split s into (0, idx-1) and (idx, num)
            split(r, s, idx);
            //save result after spliting, and new node is at the end of file
            r.write(cur, Bptfile);
            s.write(r.ptr[idx], Bptfile);
        }

        //val is small than key[idx]
        //or node is empty
        //insert into left
        if(val <= r.key[idx] || idx == r.num){
            insert(val, addr, r.ptr[idx]);
        }else{
            //insert into right part
            insert(val, addr, r.ptr[idx+1]);
        }
    }else{
        //r is leaf node
        //after the insert procedure, the node has enough space
        if(isUnique) {
            // cout<<"duplicate"<<val<<endl;
            // bpt is unique but encountered duplicate key.
            assert(r.key[idx] == val);

        }
        for(int j = r.num; j > idx; j--){
            r.key[j] = r.key[j-1];
            r.ptr[j] = r.ptr[j-1];
        }
        r.key[idx] = val;
        r.ptr[idx] = addr;
        r.num++;
        r.write(cur, Bptfile);
    }
    return 0;
}

//in: parent node, node to split, val[idx] inserts into parent
//void
template <typename T>
void bPlusTree<T>::split(Node<T> &parent, Node<T> &cur, int idx)
{
    int i;
    int half = size/2;
    for(i = parent.num; i > idx; i--){
        parent.key[i] = parent.key[i-1];
        parent.ptr[i+1] = parent.ptr[i];
    }

    Node<T> t(size);
    offset address = getNewNode();
    //insert key into parent
    parent.key[idx] = cur.key[half];
    parent.ptr[idx+1] = address;
    parent.num++;

    //copy the right half to a new node
    for(i = half + 1; i < size; i++){
        t.key[i - half - 1] = cur.key[i];
        t.ptr[i - half - 1] = cur.ptr[i];
    }

    t.num = size - half - 1;
    //the last ptr of t := the last ptr of cur
    t.ptr[t.num] = cur.ptr[size];
    //same isLeaf attribute
    t.isLeaf = cur.isLeaf;
////////////////////////////////////////////////////////////////////////////
    cur.num = half+1;//????

    if(cur.isLeaf){
        //cur.num++;//?????
        t.ptr[size] = cur.ptr[size];//point to the right part
        cur.ptr[size] = address;//last ptr of cur points to t
    }
///////////////////////////////////////////////////////////////////////////
    //sava t
    t.write(address, Bptfile);

}

template <typename T>
void bPlusTree<T>::print(offset cur)
{
    Node<T> r(size);
    r.read(cur, Bptfile);
    r.print();
    Node<T> t(size);
    for(int i = 0; i <= r.num; i++){
        t.read(r.ptr[i],Bptfile);
        if(t.num != 0)
            t.print();
        //cout<<"--------------------"<<endl;
        for(int j = 0; j <= t.num; j++){
            Node<T> s(size);
            s.read(t.ptr[j], Bptfile);
            if(s.num!=0)
                s.print();

        }
    }

}

template <typename T>
void bPlusTree<T>::printLeaf(){
    Node<T> head(size);
    head.read(root, Bptfile);
    while(!head.isLeaf){
        head.read(head.ptr[0],Bptfile);
    }
    while(1){
        for(int i = 0; i < head.num; i++){
            cout<<head.key[i]<<' ';
        }
        if(head.ptr[size] == 0){
            break;
        }
        head.read(head.ptr[size], Bptfile);
    }
    cout<<endl;
}

template <typename T>
vector<offset> bPlusTree<T>::search(T val)
{
    Node<T> r(size);
    vector<offset> result;

    r.read(root,Bptfile);
    while(r.isLeaf == false){
        int idx;
        for(idx = 0; val > r.key[idx] && idx < r.num; idx++)
        ;
        //cout<<val<<"-idx:"<<idx<<endl;
        r.read(r.ptr[idx], Bptfile);

    }
    for(int i = 0; i < r.num; i++){
        if(val == r.key[i]){
            result.push_back(r.ptr[i]);
        }
    }
    return result;
}

template <typename T>
offset bPlusTree<T>::searchInNode(T val)
{
    int i;
    Node<T> a(size);
    offset cur = root;

    do{
        a.read(cur, Bptfile);
        for(i = 0; i < a.num && val == a.key[i]; i++){
            return cur;
        }

        cur = a.ptr[i];
    }while(!a.isLeaf);

    return -1;

}



template <typename T>
void bPlusTree<T>::deleteValue(T val, offset cur)
{
    int i, j;
    Node<T> x(size);
    x.read(cur, Bptfile);
    for(i = 0; i < x.num && val > x.key[i]; i++)
    ;
    //find in current node
    if(i < x.num && val == x.key[i]){
        //find in inner node
        if(!x.isLeaf){
            Node<T> child(size);
            child.read(x.ptr[i], Bptfile);
            //child is leaf node
            if(child.isLeaf){
                if(child.num > size/2){
                    //A
                    x.key[i] = child.key[child.num - 2];
                    child.num--;

                    x.write(cur, Bptfile);
                    child.write(x.ptr[i], Bptfile);
                    return ;


                }else{
                    if(i > 0){
                        //has lsibling node
                        Node<T> lchild(size);
                        lchild.read(x.ptr[i], Bptfile);

                        if(lchild.num > size/2){
                            //B
                            for(j = child.num; j > 0; j--){
                                child.key[j] = child.key[j-1];
                                child.ptr[j] = child.ptr[j-1];
                            }

                            child.key[0] = x.key[i-1];
                            child.ptr[0] = lchild.ptr[lchild.num - 1];
                            child.num++;
                            lchild.num--;

                            x.key[i-1] = lchild.key[lchild.num - 1];
                            x.key[i] = child.key[child.num - 2];

                            x.write(cur,Bptfile);
                            lchild.write(x.ptr[i-1],Bptfile);
                            child.write(x.ptr[i], Bptfile);

                            //return ;
                        }else{
                            //C
                            for(j = 0; j < child.num; j++){
                                lchild.key[lchild.num + j] = child.key[j];
                                lchild.ptr[lchild.num + j] = child.ptr[j];
                            }
                            lchild.num += child.num;
                            lchild.ptr[size] = child.ptr[size];

                            for(j = i - 1; j < x.num - 1; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];
                            }

                            x.num--;

                            x.key[i-1] = lchild.key[lchild.num-2];

                            x.write(cur,Bptfile);
                            lchild.write(x.ptr[i-1], Bptfile);

                            i--;

                        }

                    }else{
                        //only right sibling
                        Node<T> rchild(size);
                        rchild.read(x.ptr[i+1], Bptfile);

                        if(rchild.num > size/2){
                            //D
                            x.key[i] = rchild.key[0];
                            child.key[child.num] = rchild.key[0];
                            child.ptr[child.num] = rchild.ptr[0];
                            child.num++;


                            for(j = 0; j < rchild.num - 1; j++){
                                rchild.key[j] = rchild.key[j+1];
                                rchild.ptr[j] = rchild.ptr[j+1];
                            }

                            rchild.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);
                            rchild.write(x.ptr[i+1], Bptfile);
                        }else{
                            //E
                            for(j = 0; j < rchild.num; j++){
                                child.key[child.num+j] = rchild.key[j];
                                child.ptr[child.num+j] = rchild.ptr[j];
                            }
                            child.num += rchild.num;
                            child.ptr[size] = rchild.ptr[size];

                            for(j = i; j < x.num; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];
                            }
                            x.num--;

                            x.write(cur,Bptfile);
                            child.write(x.ptr[i], Bptfile);
                        }
                    }

                }
            }else{
                //F
                offset result = searchInNode(val);
                Node<T> last(size);
                last.read(result, Bptfile);

                x.key[i] = last.key[last.num - 2];
                x.write(cur, Bptfile);


                if(child.num > size/2){
                    //H
                }else{
                    if(i > 0){
                        Node<T> lchild(size);
                        lchild.read(x.ptr[i], Bptfile);

                        if(lchild.num > size/2){
                            // I
                            for(j = child.num; j > 0; j--){
                                child.key[j] = child.key[j-1];
                                child.ptr[j+1] = child.ptr[j];
                            }

                            child.ptr[1] = child.ptr[0];
                            child.key[0] = x.key[i-1];
                            child.ptr[0] = lchild.ptr[lchild.num];

                            child.num++;
                            x.write(cur, Bptfile);
                            lchild.write(x.ptr[i-1], Bptfile);
                            child.write(x.ptr[i], Bptfile);
                        }else{
                            //J
                            lchild.key[lchild.num] = x.key[i-1];
                            lchild.num++;
                            for(j = 0; j < child.num; j++){
                                lchild.key[lchild.num+j] = child.key[j];
                                lchild.ptr[lchild.num+j] = child.ptr[j];
                            }
                            lchild.ptr[lchild.num+j] = lchild.ptr[j];
                            lchild.num += child.num;

                            for(j = i-1; j < x.num; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];
                            }
                            x.num--;

                            x.write(cur, Bptfile);
                            lchild.write(x.ptr[i], Bptfile);

                            i--;
                        }
                    }else{
                        //only right sibling
                        Node<T> rchild(size);
                        rchild.read(x.ptr[i+1], Bptfile);

                        if(rchild.num > size/2){
                            //K
                            child.key[child.num] = x.key[i];
                            child.num++;

                            child.ptr[child.num] = rchild.ptr[0];
                            x.key[i] = rchild.key[0];

                            for(j=0; j < rchild.num - 1; j++){
                                rchild.key[j] = rchild.key[j+1];
                                rchild.ptr[j] = rchild.ptr[j+1];
                            }
                            rchild.ptr[j] = rchild.ptr[j+1];
                            rchild.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);
                            rchild.write(x.ptr[i+1], Bptfile);
                        }else{
                            //L
                            child.key[child.num] = x.key[i];
                            child.num++;

                            for(j = 0; j < rchild.num; j++){
                                child.key[child.num+j] = rchild.key[j];
                                child.ptr[child.num+j] = rchild.ptr[j];
                            }
                            child.ptr[child.num+j] = rchild.ptr[j];
                            child.num += rchild.num;

                            for(j = i; j < x.num; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];

                            }
                            x.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);

                        }
                    }
                }

            }
            deleteValue(val, x.ptr[i]);

        }else{
            //G
            for(j = i; j < x.num - 1; j++){
                x.key[j] = x.key[j+1];
                x.ptr[j] = x.ptr[j+1];

            }
            x.num--;
            x.write(cur, Bptfile);
            return ;
        }

    }else{
        if(!x.isLeaf){
            Node<T> child(size);
            child.read(x.ptr[i], Bptfile);

            if(!child.isLeaf){
                if(child.num > size/2){

                }else{

                    if(i > 0){
                        Node<T> lchild(size);
                        lchild.read(x.ptr[i], Bptfile);

                        if(lchild.num > size/2){
                            // I
                            for(j = child.num; j > 0; j--){
                                child.key[j] = child.key[j-1];
                                child.ptr[j+1] = child.ptr[j];
                            }

                            child.ptr[1] = child.ptr[0];
                            child.key[0] = x.key[i-1];
                            child.ptr[0] = lchild.ptr[lchild.num];

                            child.num++;
                            x.write(cur, Bptfile);
                            lchild.write(x.ptr[i-1], Bptfile);
                            child.write(x.ptr[i], Bptfile);
                        }else{
                            //J
                            lchild.key[lchild.num] = x.key[i-1];
                            lchild.num++;
                            for(j = 0; j < child.num; j++){
                                lchild.key[lchild.num+j] = child.key[j];
                                lchild.ptr[lchild.num+j] = child.ptr[j];
                            }
                            lchild.ptr[lchild.num+j] = lchild.ptr[j];
                            lchild.num += child.num;

                            for(j = i-1; j < x.num; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];
                            }
                            x.num--;

                            x.write(cur, Bptfile);
                            lchild.write(x.ptr[i], Bptfile);

                            i--;
                        }
                    }else{
                        //only right sibling
                        Node<T> rchild(size);
                        rchild.read(x.ptr[i+1], Bptfile);

                        if(rchild.num > size/2){
                            //K
                            child.key[child.num] = x.key[i];
                            child.num++;

                            child.ptr[child.num] = rchild.ptr[0];
                            x.key[i] = rchild.key[0];

                            for(j=0; j < rchild.num - 1; j++){
                                rchild.key[j] = rchild.key[j+1];
                                rchild.ptr[j] = rchild.ptr[j+1];
                            }
                            rchild.ptr[j] = rchild.ptr[j+1];
                            rchild.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);
                            rchild.write(x.ptr[i+1], Bptfile);
                        }else{
                            //L
                            child.key[child.num] = x.key[i];
                            child.num++;

                            for(j = 0; j < rchild.num; j++){
                                child.key[child.num+j] = rchild.key[j];
                                child.ptr[child.num+j] = rchild.ptr[j];
                            }
                            child.ptr[child.num+j] = rchild.ptr[j];
                            child.num += rchild.num;

                            for(j = i; j < x.num; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];

                            }
                            x.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);
                        }
                    }
                }

            }else{
                if(child.num > size/2){
                    //M
                }else{
                    if(i > 0){
                        Node<T> lchild(size);
                        lchild.read(x.ptr[i], Bptfile);

                        if(lchild.num > size/2){
                            //N
                            for(j=child.num; j > 0; j--){
                                child.key[j] = child.key[j-1];
                                child.ptr[j] = child.ptr[j-1];
                            }
                            child.key[0] = x.key[i-1];
                            child.ptr[0] = lchild.ptr[lchild.num-1];
                            child.num++;
                            lchild.num--;

                            x.key[i-1] = lchild.key[lchild.num-1];

                            lchild.write(x.ptr[i-1], Bptfile);
                            child.write(x.ptr[i], Bptfile);
                            x.write(cur, Bptfile);


                        }else{
                            //O
                            for(j = 0; j < child.num; j++){
                                lchild.key[lchild.num+j] = child.key[j];
                                lchild.ptr[lchild.num+j] = child.ptr[j];

                            }
                            lchild.num += child.num;

                            for(j = i; j < x.num - 1; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j+1] = x.ptr[j+2];
                            }
                            x.num--;

                            lchild.write(x.ptr[i-1], Bptfile);
                            x.write(cur, Bptfile);
                            i--;


                        }
                    }else{
                        Node<T> rchild(size);
                        rchild.read(x.ptr[i+1], Bptfile);

                        if(rchild.num > size/2){
                            //P
                            x.key[i] = rchild.key[0];
                            child.key[child.num] = rchild.key[0];
                            child.ptr[child.num] = rchild.ptr[0];
                            child.num++;

                            for(j = 0; j < rchild.num-1; j++){
                                rchild.key[j] = rchild.key[j+1];
                                rchild.ptr[j] = rchild.ptr[j+1];
                            }
                            rchild.num--;

                            x.write(cur, Bptfile);
                            rchild.write(x.ptr[i+1], Bptfile);
                            child.write(x.ptr[i], Bptfile);
                        }else{
                            //Q
                            for(j = 0; j < rchild.num; j++){
                                child.key[child.num+j] = rchild.key[j];
                                child.ptr[child.num+j] = rchild.ptr[j];

                            }
                            child.num += rchild.num;
                            child.ptr[size] = rchild.ptr[size];


                            for(j = 0; j < x.num-1; j++){
                                x.key[j] = x.key[j+1];
                                x.ptr[j] = x.ptr[j+1];
                            }
                            x.num--;

                            x.write(cur, Bptfile);
                            child.write(x.ptr[i], Bptfile);


                        }
                    }
                }

            }
            deleteValue(val, x.ptr[i]);

        }

    }

}

#endif
