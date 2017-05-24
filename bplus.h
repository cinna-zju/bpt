#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#define size 5
using namespace std;

typedef long offset;


template <typename T>
class Node{
public:
    int num;//num of
    T *key;
    offset *ptr;
    bool isLeaf;

    Node(){
        // size = (4096 - sizeof(int) - sizeof(bool) - sizeof(offset))
        //             /(sizeof(T) + sizeof(offset));
        key = (T*)malloc(sizeof(T)*size);
        ptr = (offset*)malloc(sizeof(offset)*size);

    }

    void write(offset addr, FILE* fp){
        fseek(fp, addr, SEEK_SET);
        fwrite(&num, sizeof(int), 1, fp);
        fwrite(&isLeaf, sizeof(bool), 1, fp);
        fwrite(key, sizeof(T), size, fp);
        fwrite(ptr, sizeof(offset), size+1, fp);
    };

    void read(offset addr, FILE* fp){
        fseek(fp, addr, SEEK_SET);
        fread(&num, sizeof(int), 1, fp);
        fread(&isLeaf, sizeof(bool), 1, fp);
        fread(key, sizeof(T), size, fp);
        fread(ptr, sizeof(offset), size+1, fp);
    };

    void print(){
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

};


template <typename T>
class bPlusTree{
public:
    string fileName;
    offset root;//根在文件内的偏移量
    FILE *Bptfile;
    FILE *RecFile;

    void init(){
        root = newNode();
        //cout<<"root addr:"<<root<<endl;
        Node<T> r;
        r.ptr[size] = 0;
        r.num = 0;
        r.isLeaf = true;
        r.write(root, Bptfile);
    };

    offset newNode(){
        fseek(Bptfile, 0, SEEK_END);
        return ftell(Bptfile);
    };

public:
    bPlusTree(string);
    ~bPlusTree(){fclose(Bptfile);};

    void insert(T, offset, offset);
    void insert(T, offset);
    void split(Node<T>&, Node<T>&, int);
    void print(offset);
    void printLeaf();


    pair<offset, offset> search(T, Node<T>);


};

template <typename T>
bPlusTree<T>::bPlusTree(string filename)
{
    fileName = filename;

    if((Bptfile = fopen(fileName.c_str(), "wb+"))){
        cout << "file opened" << endl;
        init();
    }else{
        cout << "file error" << endl;
        exit(-1);
    }


}

template <typename T>
void bPlusTree<T>::insert(T val, offset addr){

    Node<T> ro;
    ro.read(root, Bptfile);
    if(ro.num == size){

        Node<T> nro;
        nro.num = 0;
        nro.isLeaf = false;
        nro.ptr[0] = root;

        split(nro, ro, 0);

        ro.write(root, Bptfile);
        root = newNode();
        nro.write(root, Bptfile);
        //nro.print();
    }
    insert(val, addr, root);


}

template <typename T>
void bPlusTree<T>::insert(T val, offset addr, offset cur){

    int idx;
    Node<T> r;
    r.read(cur, Bptfile);

    for(idx = 0; idx < r.num && r.key[idx] < val; idx++)
        ;

    if(idx < r.num && r.isLeaf && r.key[idx] == val){
        cout << "duplicate key" <<endl;
        return;
    }
    if( !r.isLeaf ){
        Node<T> s;
        s.read(r.ptr[idx], Bptfile);

        if(s.num == size){
            split(r, s, idx);
            r.write(cur, Bptfile);
            s.write(r.ptr[idx], Bptfile);
        }

        if(val < r.key[idx] || idx == r.num){
            insert(val, addr, r.ptr[idx]);
        }else{
            insert(val, addr, r.ptr[idx+1]);
        }
    }else{
        for(int j = r.num; j > idx; j--){
            r.key[j] = r.key[j-1];
            r.ptr[j] = r.ptr[j-1];
        }
        r.key[idx] = val;
        r.ptr[idx] = addr;
        r.num++;
        r.write(cur, Bptfile);
    }
}



template <typename T>
void bPlusTree<T>::split(Node<T> &parent, Node<T> &cur, int childNum)
{
    int i;
    int half = size/2;
    for(i = parent.num; i > childNum; i--){
        parent.key[i] = parent.key[i-1];
        parent.ptr[i+1] = parent.ptr[i];
    }
    parent.num++;

    Node<T> t;

    offset address = newNode();

    parent.key[childNum] = cur.key[half+1];
    parent.ptr[childNum+1] = address;

    for(i = half + 1; i < size; i++){
        t.key[i - half - 1] = cur.key[i];
        t.ptr[i - half - 1] = cur.ptr[i];
    }

    t.num = size - half - 1;
    t.ptr[t.num] = cur.ptr[size];
    t.isLeaf = cur.isLeaf;

    cur.num = half;

    if(cur.isLeaf){
        cur.num++;
        t.ptr[size] = cur.ptr[size];
        cur.ptr[size] = address;
    }

    t.write(address, Bptfile);

}

template <typename T>
void bPlusTree<T>::print(offset cur)
{
    Node<T> r;
    r.read(cur, Bptfile);
    r.print();
    Node<T> t;
    for(int i = 0; i <= r.num; i++){
        printf("(%d, %d)\n", 1, i);
        t.read(r.ptr[i],Bptfile);
        if(t.num != 0)
            t.print();

        Node<T> s;
        for(int j = 0; j <= t.num; j++){
            printf("(%d, %d)\n", 2, j);
            s.read(t.ptr[j], Bptfile);
            if(s.num != 0)
                s.print();
        }
        cout<<"--------------------"<<endl;
    }

}

template <typename T>
void bPlusTree<T>::printLeaf(){
    Node<T> head;
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
