#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#define MAX_SIZE 5
using namespace std;

typedef long offset;


template <typename T>
class Node{
public:
    int num;//num of
    int size;
    T *key;
    offset *ptr;
    bool isLeaf;

    Node():num(0), isLeaf(false){
        // size = (4096 - sizeof(int) - sizeof(bool) - sizeof(offset))
        //             /(sizeof(T) + sizeof(offset));
        size = 5;
        key = (T*)malloc(sizeof(T)*size);
        ptr = (offset*)malloc(sizeof(offset)*size);
    }
    void write(offset addr, FILE* fp){
        fseek(fp, addr, SEEK_SET);
        fwrite(&size, sizeof(int), 1, fp);
        fwrite(&num, sizeof(int), 1, fp);
        fwrite(key, sizeof(T), size, fp);
        fwrite(ptr, sizeof(offset), size+1, fp);
    };

    void read(offset addr, FILE* fp){
        fseek(fp, addr, SEEK_SET);
        fread(&size, sizeof(int), 1, fp);
        fread(&num, sizeof(int), 1, fp);
        fread(key, sizeof(T), size, fp);
        fread(ptr, sizeof(offset), size+1, fp);
    };

    void print(){
        cout<<size<<" "<<num<<endl;
        for(int i = 0; i < size; i++){
            cout<<key[i]<<' ';
        }
        cout<<endl;

        for(int i = 0; i < size+1; i++){
            cout<<ptr[i]<<' ';
        }
        cout<<endl;
    }

    void insert(T val , offset addr, int idx){
        for(int i = idx; i < size-1; i++){
            key[i+1] = key[i];
            ptr[i+2] = ptr[i+1];
        }
        key[idx] = val;
        ptr[idx+1] = addr;
    }

};


template <typename T>
class bPlusTree{
public:
    string fileName;
    offset root;//根在文件内的偏移量
    FILE *Bptfile;
    FILE *RecFile;
    void read(offset root){
        Node<T> r;
        r.read(root, Bptfile);
    };

    void write(Node<T> &r){
        r.write(0, Bptfile);
    }
    void init(){Node<T> r; write(r);};
    void print(offset cur){
        Node<T> r;
        r.read(cur, Bptfile);
        r.print();
    }
    offset newNode();

public:
    bPlusTree(string);
    ~bPlusTree(){fclose(Bptfile);};

    void insert(T, offset);
    void split(Node<T>&, Node<T>&, int);

    pair<offset, offset> search(T, Node<T>);


};

template <typename T>
bPlusTree<T>::bPlusTree(string filename)
{
    root = 0;
    fileName = filename;

    if( (Bptfile = fopen(fileName.c_str(), "rb+")) == false ){
        init();
        Bptfile = fopen(fileName.c_str(), "rb+");
    }


}

template <typename T>
pair<offset, offset> bPlusTree<T>::search(T val, Node<T> C)
{
    offset cursor = root;

    int i;
    do{
        read(cursor, C);
        for(i = 0; i < C.num && val > C.key[i]; i++)
            ;
        if(i < C.num && C.isLeaf == true && val == C.key[i]){
            return make_pair(cursor, C.ptr[i]);//
        }

    }while(C.isLeaf == false);
    return make_pair(0, 0);



}

template <typename T>
void bPlusTree<T>::insert(T val, offset addr)
{
    Node<T> r;
    r.read(root, Bptfile);
    int idx;
    for(idx = 0; idx < r.size; idx++){
        if(val < r.key[idx]) break;
    }
    cout<<"idx:"<<idx<<endl;
    r.insert(val, addr, idx-1);
    r.write(root, Bptfile);
}


template <typename T>
void bPlusTree<T>::split(Node<T> &parent, Node<T> &current, int childNum)
{

}

// template <typename T>
// int insertInLeaf(Node L, T val, Node<T>* ptr)
// {
//     if(K < L.key[0]){
//         //insert P,K into L just before L.P0
//         L.key.insert(L.key.begin(), val);
//         L.ptr.insert(L.ptr.begin(), ptr);
//     }else{
//         for(vector<T>::const_iterator i = L.key.begin();
//                 i != L.key.end(); i++){
//             if(val <= (*i)) continue;
//             else{
//                 i--;
//                 L.key.insert(i, val);
//                 L.ptr.insert(i, ptr);
//             }
//         }
//     }
//     return 0;
//
// }
//
// template <typename T>
// int insertInParent(Node N, T Knew, Node Nnew)
// {
//     if(N.isRoot == true){
//         Node R;
//         R.key.push_back(Knew);
//         R.ptr.push_back(&N);
//         R.ptr.push_back(&Nnew);
//         R.isRoot = true;
//         N.isRoot = false;
//     }
//
//     Node<T> P = *N.parent;
//     if(P.ptr.size() < n){
//         N.key.push_back(Knew);
//         N.ptr.push_back(&Nnew);
//     }else{
//         Node<T> T = P;
//         T.key.push_back(Knew);
//         T.ptr.push_back(&Nnew);
//         //clear P
//         vector<T>().swap(P.key);
//         vector<Node*>().swap(P.ptr);
//
//         Node<T> Pnew;
//         for(int i = 0; i <= n/2; i++){
//             P.ptr.push_back(T.ptr[i]);
//         }
//
//         T Knewnew = T.key[n/2];
//         for(int i = n/2+1; i <= n+1; i++){
//             Pnew.ptr.pushback(T.ptr[i]);
//         }
//         insertInParent(P, Knewnew, Pnew);
//
//
//     }
//
// }
//search()
// while(C.isLeaf == false){
//
//     for(vector<T>::const_iterator i = C.key.begin();
//         i != C.key.end(); i++, cnt++){
//         if(val <= (*i)) {
//             if(i+1 == C.key.end()){
//                 C = C.ptr.back();
//             }else{
//                 if(val == (*i)){
//                     C = *(C.ptr.at(cnt+1));
//                 }else{
//                     C = *(C.ptr.at(cnt));
//                 }
//             }
//
//         }
//     }
// }
//
// for(vector<T>::const_iterator i = C.key.begin(); i != C.key.end(); i++){
//     if(val == (*i)) return make_pair(C, (*i));
// }
// return NULL;


//insert
// if(this->isEmpty()){
//
//     this->root = new Node();
// }else{
//     pair<Node, T> result = search(val);
//     Node L = result.first;
//     if(leaf.size() < n - 1){
//         insertInLeaf(L, val, ptr);
//     }else{
//         Node Lnew  = new Node();
//         Node T = L;
//         insertInLeaf(T, val, ptr);
//         Lnew.ptr[n] = &L;
//         L.ptr[n] = &Lnew;
//         for(int i = 0; i < n-1; i++){
//             if(i <= n/2){
//                 L.ptr[i] = T.ptr[i];
//                 L.key[i] = T.key[i];
//             }else{
//                 L.ptr[i] = NULL;
//                 L.key[i] = 0;
//                 Lnew.ptr[i] = T.ptr[i];
//                 Lnew.key[i] = T.key[i];
//             }
//         }
//         T Knew = Lnew.key[0];
//         insertInParent(L, Knew, Lnew);
//
//     }
// }
// return 0;
