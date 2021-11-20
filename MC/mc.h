#ifndef _MC_H
#define _MC_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
using namespace std;

class MC{
private:
    struct BUCKET_node {
        uint32_t key;
        uint32_t count;
 //       int nvote;
    };
    BUCKET_node *bucket;
    BOBHash32 *hash[ROW_NUM];
    int row;
    int width;
    int key_len = 4;
    float beta = 1.0;
    float alpha = 1.0;
//    int max_nvote = 0;

    void Clear(){
        for(int i =0;i<row*width;i++){
             bucket[i].count  = 0;
             bucket[i].key = 0;
        }
    }
public:
    MC(int total_memory, float a, float b){
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
        cout<<"MC"<<endl<<"\t bucket number: "<<width*row<<"\t width: "<<width<<endl;
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

                if(rand()/double(RAND_MAX)<pow((beta/(bucket[pos].count & 0x00ffffff)), alpha)){

                    if((bucket[pos].count>>31)  == 0){//nvote still exist rand()/double(RAND_MAX)<pow((beta/(bucket[pos].count & 0x0fff)

                        //bucket[pos].count = ((bucket[pos].count >> 24) + 1)<<24 + (bucket[pos].count & 0x00ffffff);
                        bucket[pos].count += 0x01000000;//nvote

                        if((bucket[pos].count>>24 ) >= (bucket[pos].count &0x00ffffff)){// replace

                            bucket[pos].key = key;
                            bucket[pos].count = 1;
                        }else{
                            if((bucket[pos].count >>24)> 100){
                            //    max_nvote +=1;
                                bucket[pos].count = bucket[pos].count | 0x80000000;
                            }
                        }

                    }
                }
            }

        }
    }

    map<uint32_t, int> Query_threshold(uint32_t threshold){
        map<uint32_t,int> result;
        set<uint32_t> res;
        for(int i =0;i<width*row;i++){
           // max_nvote = max(max_nvote,bucket[i].nvote);
           // if(bucket[i].count &0x8000 !=0){// flag is 1

          //  }
         //   cout<<"cout: "<<(bucket[i].count & 0x00ffffff)<<"\t nvote: "<<(bucket[i].count >> 24)<<endl;
            if((bucket[i].count & 0x00ffffff) > threshold) res.insert(bucket[i].key);
        }
        uint32_t res_val = 0;
        int index;
        int pos;
        uint32_t tmp_value;
        for(auto it = res.begin();it!=res.end();it++){
            res_val= 0;
            for(int i=0;i<row;i++){
                index = (hash[i]->run((const char*)&(*it),key_len))%width;
                pos = i*width + index;
                if(bucket[pos].key == *it){
                    if((bucket[pos].count>>31) == 0){
                        tmp_value = bucket[pos].count & 0x00ffffff;
                    }else{
                        tmp_value = bucket[pos].count & 0x00ffffff;
                    }
                    res_val = std::max(res_val, tmp_value);
                }

            }
            result.insert(make_pair(*it, res_val));
        }
    //    cout<<"max nvote is: "<<max_nvote<<endl;
        return result;
    }
};

#endif
