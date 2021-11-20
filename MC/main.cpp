#include "../common/traceRead.h"
#include "../common/param.h"
#include "./mc.h"
using namespace std;

uint32_t insert_data[MAX_PACKET];
unordered_map<uint32_t,int> benchmark_data;

int main()
{
    int packet_num;
    int total_memory, threshold;
    float alpha;
    float beta;
    char path[100] = "../data/2002.txt";
    alpha = 1.0;
    beta =1.0;
        /******for python call*******/
    /*char path[100];
    cout<<"input data path:(for example: ../data/wide_100000_s10.txt"<<endl;
    cin >> path;


    cout<<"Input alpha and beta (for example 0.9 5)"<<endl;
    cin >> alpha>> beta;
    */
     total_memory = 16*1024;
    threshold = 1000;
    packet_num = TraceRead(path,insert_data,benchmark_data);

    timespec start_time, end_time;
    double insert_throughput;
    long long mcsketch_timediff;
    map<uint32_t,int> mc_thres;

    vector<pair<uint32_t,int>> real_threshold_result;
    for(auto it:benchmark_data){
        if(it.second>threshold){
            real_threshold_result.push_back( make_pair(it.first,it.second));
        }
    }
    cout<<"real number of flows exceeding the threshold "<<threshold<<": "<<real_threshold_result.size()<<endl;

    /****************************3. HH3*******************************/
    MC *mcsketch = new MC(total_memory, alpha, beta);
    mcsketch->Print_basic_info();

    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i =0;i<packet_num;i++){
        mcsketch->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    mcsketch_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);

    mc_thres = mcsketch->Query_threshold(threshold);

    /**********************************result****************************/
    ofstream outFile;
    outFile.open("result.csv",ios::app);
    cout<<"**************************RESULT***********************"<<endl;
    int cnt = 0;
    int tp = 0;
    float ab_error = 0;
    float re_error = 0;
    float precision;
    float recall;
    float avg_ab_error;
    float avg_re_error;
    float F1;

    /*********************************************HH3**************/
    cout <<"*******RESULT OF HH3******"<<endl;
    cnt = 0;
    tp = 0;
    ab_error = 0;
    re_error = 0;
    for(auto it=mc_thres.begin();it!=mc_thres.end();it++){
        cnt++;
        for(auto ik=real_threshold_result.begin();ik != real_threshold_result.end();ik++){
            if(ik->first == it->first){
                tp++;
                ab_error += abs(it->second - ik->second);
                re_error += abs((it->second - ik->second)*1.0/ik->second);
            }
        }
    }
    precision = tp*1.0/cnt;
    recall = tp*1.0/real_threshold_result.size();
    F1 = 2*precision*recall/(precision+recall);
    avg_ab_error = ab_error*1.0 /tp;
    avg_re_error = re_error*1.0/ tp;
    insert_throughput = (double)1000.0*packet_num/mcsketch_timediff;
    cout<<"precision: "<<precision<<endl<<"recall: "<<recall<<endl<<"F1 score: "<<F1<<endl;
    cout<<"average absolute error: "<<avg_ab_error<<endl<<"average relative error: "<<avg_re_error<<endl;
    cout<<"insert throughput: " <<insert_throughput<<endl;
    outFile<<path<<","<<alpha<<","<<beta<<",MCSketch,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;


}
