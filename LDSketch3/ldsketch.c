#include "./ldsketch.h"
uint32_t LDSketch::max_nbucket = 0;
uint32_t LDSketch::max_length = 0;
uint32_t LDSketch::T = 0;

//uint32_t BUCKET::BucketUpdate
uint32_t BUCKET::BucketUpdate(uint32_t cur_key, uint32_t cur_nbucket){
        sum +=1;
        // new packet is stored in array
        if(array.find(cur_key)!= array.end()){
            array[cur_key] += 1;
        }
        // array size is smaller than length
        else if(array.size() < length){
            array.insert(pair<uint32_t,uint32_t>(cur_key,1));
        } else{
            uint32_t k = sum / LDSketch::T;
            if(((k+1)*(k+2)-1)<= length) {
                // get error
                uint32_t tmp_min = 0;
                for(auto iter = array.begin();iter!= array.end();iter++){
                    if(iter == array.begin())    tmp_min = iter->second;
                    else{
                        if(iter->second < tmp_min){
                            tmp_min = iter->second;
                        }
                    }
                }
                if( 1< tmp_min) tmp_min =1;
                error += tmp_min;
                //delete array which value is 1
                for(auto iter = array.begin();iter!=array.end();){
                    iter-> second -= tmp_min;
                    if(iter->second == 0){
                        auto tmp_iter = next(iter);
                        array.erase(iter);
                        iter = tmp_iter;
                    }else{
                        iter++;
                    }
                }
                if(1 > tmp_min){
                    array.insert(pair<uint32_t,uint32_t>(cur_key,1-tmp_min));
                }
             }

            else if(cur_nbucket < LDSketch::max_nbucket){
                uint32_t prev_length = length;
                length = (k+1)*(k+2) -1;
                if(length >LDSketch::max_length){
                    length = LDSketch::max_length;
                }
                uint32_t delta_length = length - prev_length;
                if(delta_length > LDSketch::max_nbucket - cur_nbucket){
                    length = prev_length + LDSketch::max_nbucket - cur_nbucket;
                    delta_length = LDSketch::max_nbucket - cur_nbucket;
                }
                array.insert(pair<uint32_t,uint32_t>(cur_key,1));
                return delta_length;
            }
        }
        return 0;
}

uint32_t BUCKET::BucketEstimate(uint32_t cur_key){
    uint32_t result = 0;
    if(array.size()> 0){
        if(array.find(cur_key) != array.end()){
            result = array[cur_key];
        }
    }
    result += error;
    return result;
}
LDSketch::LDSketch(int total_memory, int threshold, float ratio){
        row = ROW_NUM;
        uint32_t bucket_size = 4+4+4;//<sum,error,length>
        uint32_t bucket_num = total_memory * ratio /bucket_size;
        width = floor(bucket_num/row);
        max_nbucket = total_memory*(1-ratio)/8;// <key,value> size is 8 byte;
        max_length = 10;
        T = threshold;
        for(int i =0;i>row;i++) hash[i] = new BOBHash32(i+750);
}

void LDSketch::Insert(uint32_t key){
        int index;
        int pos;
        for (int i =0;i<row;i++){
            index = (hash[i]->run((const char*)&key,key_len))%width;
            pos = i*width + index;
  //          BUCKET* tmpbkt = &bucket[pos];
            uint32_t delta = bucket[pos].BucketUpdate(key,cur_nbucket);
            cur_nbucket += delta;
        }
 }

void LDSketch::Print_basic_info(){
    cout<<"LDSketch"<<endl<<"\t bucket number: "<<width*row<<"\t width: "<<width<<endl;
    cout<<"\t total cell number: "<<max_nbucket<<endl;
}

uint32_t LDSketch::Estimate(uint32_t cur_key){
    uint32_t result = 0;
    int index;
    int pos;
    for (int i = 0;i<ROW_NUM;i++){
        index = (hash[i]->run((const char*)&cur_key,key_len))%width;
        pos = i*width + index;
        uint32_t tmp_res = bucket[pos].BucketEstimate(cur_key);
        if(i==0||tmp_res<result)
            result = tmp_res;
    }
}

map<uint32_t,int> LDSketch::Query_threshold(int threshold){
    map<uint32_t,int> result;
    set<uint32_t> res;
    for(int i =0;i<width*row;i++){
        for(uint32_t j =0;j<bucket[i].array.size();j++){
            cout<<"test"<<endl;
        }
    }
}
