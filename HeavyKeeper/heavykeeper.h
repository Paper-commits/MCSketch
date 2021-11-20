 #ifndef _HEAVYKEEPER_H
#define _HEAVYKEEPER_H

#include "../common/param.h"
#include "../common/BOBHash32.h"
using namespace std;

#define HK_d 2
#define HK_b 1.08
//#define d 2

class HeavyKeeper
{
private:
  //  int d =2;

    struct node{
        int C;//Count
        uint32_t FP;//fingerprint
    };
    node *bucket;

    BOBHash32 *hash[ROW_NUM];
    int w;
    int row_size;

    uint32_t heap_size = 0;
    int key_len = 4;//4 byte;


public:
    HeavyKeeper(int total_memory){
        w = floor(total_memory/sizeof(node));
        row_size = floor(w/ROW_NUM);
        bucket = new node[w];
        for(int i =0;i<ROW_NUM;i++)
            hash[i] = new BOBHash32(i+750);
    }

    void Print_basic_info()
    {
        cout<<"HeavyKeeper"<<endl<<"\t bucket number: "<<w<<"\t width: "<<row_size<<endl;
 //       cout<<"HeavyKeeper"<<endl<<"heap memory: "<<heap_memory<<"B"<<endl;
 //       CM_print_basic_info();
    }
    void Insert(uint32_t pkt){
        int hash_index;
        int tmp_count;
        for(int i =0;i<ROW_NUM;i++){
            hash_index = row_size*i + (hash[i]->run((const char*)&pkt,key_len))%row_size;
            tmp_count= bucket[hash_index].C;
            if(bucket[hash_index].FP == pkt){
                bucket[hash_index].C++;
            }else{
   //             if(!(rand()%int(pow(HK_b,bucket[hash_index].C)))){
                if((rand()*(1.0/RAND_MAX))< pow(HK_b,-bucket[hash_index].C)){
                    bucket[hash_index].C --;
                    if(bucket[hash_index].C <=0){
                        bucket[hash_index].C = 1;
                        bucket[hash_index].FP = pkt;
                    }
                }
            }
        }
    }

    map<uint32_t,int> Query_threshold(int threshold){
        map<uint32_t,int> result;
        set<uint32_t> res;
        for(int i =0;i<row_size*ROW_NUM;i++){
            if(bucket[i].C > threshold){
                uint32_t tmp_key = bucket[i].FP;
                uint32_t pos;
                int max_count = 0;
                for(int j=0;j<ROW_NUM;j++){
                    pos = row_size*j + (hash[j]->run((const char*)&tmp_key,key_len))%row_size;
                    if(bucket[pos].FP == tmp_key && bucket[pos].C > max_count)
                        max_count = bucket[pos].C;
                }
                if(result.find(tmp_key)==result.end())
                    result.insert(make_pair(tmp_key,max_count));
            }
        }

        return result;
    }


};

#endif
