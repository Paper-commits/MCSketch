#ifndef _HH3_H
#define _HH3_H

#include "../../common/param.h"
#include "../../common/BOBHash32.h"
using namespace std;

class HH3{
private:
    struct BUCKET_node {
        uint32_t key;
        int count;
        int nvote;
    };
    BUCKET_node *bucket;
    BOBHash32 *hash[ROW_NUM];
    int row;
    int width;
    int key_len = 4;
    float beta = 1.0;
    float alpha = 1.08;

    void Clear(){
        for(int i =0;i<row*width;i++){
             bucket[i].count = bucket[i].nvote = 0;
             bucket[i].key = 0;
        }
    }
public:
    HH3(int total_memory, float a, float b){
        alpha = a;
        beta = b;
        row = ROW_NUM;
        width = floor(total_memory/(sizeof(BUCKET_node)*row));
        bucket = new BUCKET_node[row*width];
        for(int i=0;i<row;i++)
            hash[i] = new BOBHash32(i+750);
        Clear();
    }

    void Print_basic_info(){
        cout<<"HH3"<<endl<<"\t bucket number: "<<width*row<<"\t width: "<<width<<endl;
    }

    void Insert(uint32_t key, int f=1){
        int index;
        int pos;
        for(int i=0;i<row;i++){
            index = (hash[i]->run((const char*)&key,key_len))%width;
            pos = i*width + index;
            if(bucket[pos].key == 0){//bucket is null
                bucket[pos].key = key;
                bucket[pos].count = f;
            } else if(bucket[pos].key == key){// new flow is equal the stored flow
                bucket[pos].count += f;
            } else{
     //           if((rand()/double(RAND_MAX))<pow(base,-bucket[pos].count)){
                if((rand()/double(RAND_MAX))< pow((beta/bucket[pos].count),alpha)){
                    bucket[pos].nvote += f;

                    if(bucket[pos].nvote >  bucket[pos].count){//replace
                        bucket[pos].key = key;
                        bucket[pos].count = 1;
                        bucket[pos].nvote = 0;
                    }
                }
            }

        }
    }

    map<uint32_t, int> Query_threshold(int threshold){
        map<uint32_t,int> result;
        set<uint32_t> res;
        for(int i =0;i<width*row;i++){
            if(bucket[i].count > threshold) res.insert(bucket[i].key);
        }
        int res_val = 0;
        int index;
        int pos;

        for(auto it = res.begin();it!=res.end();it++){
            res_val= 0;
            for(int i=0;i<row;i++){
                index = (hash[i]->run((const char*)&(*it),key_len))%width;
                pos = i*width + index;
                if(bucket[pos].key == *it)
                    res_val = std::max(res_val, bucket[pos].count);
            }
            result.insert(make_pair(*it, res_val));
        }
        return result;
    }
};

#endif
