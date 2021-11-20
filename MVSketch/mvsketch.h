#ifndef _MVSKETCH_H
#define _MVSKETCH_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
#include <set>
using namespace std;

class MVSketch {
private:
    struct BUCKET_node {
        int sum;
        int count;
        uint32_t key;
    };
    BUCKET_node *bucket;// the data structure of MVSketch

    BOBHash32 *hash[ROW_NUM];
    int row;
    int width;
    int key_len = 4;//4 byte

    void Clear(){
        for(int i=0;i<row*width;i++)
            bucket[i].sum = bucket[i].count = bucket[i].key = 0;
    }

public:
    MVSketch(int total_memory){
        row = ROW_NUM;
        width = floor(total_memory/(sizeof(BUCKET_node)*row));
  //      cout<<"bucket size: "<<sizeof(BUCKET_node)<<endl;
        bucket = new BUCKET_node[row*width];
        for(int i=0;i<row;i++)
            hash[i] = new BOBHash32(i+750);
        Clear();
    }

    void Print_basic_info(){
        cout<<"MVSketch"<<endl<<"\t bucket number: "<<row*width<<"\t width: "<<width<<endl;
    }

    void Insert(uint32_t key, int f = 1){// the default value is 1
        int index;
        int pos;
        for(int i=0;i<row;i++){
            index = (hash[i]->run((const char*)&key,key_len))%width;
            pos = i*width + index;
            bucket[pos].sum += f;
            if (bucket[pos].key == 0){// if the bucket is null
                bucket[pos].count = f;
                bucket[pos].key = key;
            }else if(bucket[pos].key == key){//if the packet belong to the flow in the bucket
                bucket[pos].count += f;
            } else{
                bucket[pos].count -= f;
                if(bucket[pos].count<=0){// Majority vote algorithm
                    bucket[pos].key = key;
                //    bucket[pos].count = -bucket[pos].count;
                    bucket[pos].count = 0;
                }
            }
        }
    }

    map<uint32_t,int> Query_threshold(int threshold){
        map<uint32_t,int> result;
        set<uint32_t> res;
        for(int i=0;i<width*row;i++){
            if(bucket[i].sum > threshold) res.insert(bucket[i].key);
        }
        int res_val;

        for(auto it = res.begin(); it!=res.end();it++){
            int temp_val=0;
            int index;
            int pos;
            for(int i=0;i<ROW_NUM;i++){
                index = (hash[i]->run((const char*)&(*it),key_len))%width;
   //             cout<<*it<<endl;
                pos = i*width + index;
                if(bucket[pos].key == *it){
                    temp_val = (bucket[pos].sum + bucket[pos].count) / 2;
                } else{
                    temp_val = (bucket[pos].sum - bucket[pos].count) / 2;
                }
                if(i==0) res_val = temp_val;
                else res_val = std::min(temp_val,res_val);
            }
            if(res_val > threshold){
                result.insert(make_pair(*it,res_val));
            }

        }
        return result;
    }

};
#endif
