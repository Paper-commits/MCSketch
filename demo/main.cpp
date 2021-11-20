#include "../common/traceRead.h"
#include "../common/param.h"
#include "../MVSketch/mvsketch.h"

#include "../LDSketch3/ldsketch.h"
#include "../WavingSketch/wavingsketch.h"
#include "../HeavyKeeper/heavykeeper.h"
#include "../MC/mc.h"
using namespace std;

uint32_t insert_data[MAX_PACKET];
unordered_map<uint32_t,int> benchmark_data;

int main()
{
    int packet_num;
    int total_memory, threshold;
//    char path[100] = "../data/wide_10000000.txt";
    /*******for debug**********/
//    /char path[100] = "../data/s1.14_10000000.txt";

    /******for python call*******/
    char path[100];
    cout<<"input data path:(for example: ../data/1.txt"<<endl;
    cin >> path;

    cout<<"Input memory(KB) and threshold (for example: 4 1000)"<<endl;
    cin >>total_memory>>threshold;
    packet_num = TraceRead(path, insert_data, benchmark_data);

    total_memory = 1024*total_memory;
    cout<<"test trace read:"<<endl;
    cout<<"packet number: "<<packet_num<<endl;
    cout<<"total memory(Byte): "<<total_memory<<"\t threshold: "<<threshold<<endl;
    double insert_throughput;

    timespec start_time, end_time;
    long long mvsketch_timediff,hhsketch_timediff,hhsketch2_timediff,ldsketch_timediff,mcsketch_timediff;
    long long spacesaving_timediff, wavingsketch_timediff,heavykeeper_timediff,hhsketch3_timediff;

    map<uint32_t,int> mv_thres;
    map<uint32_t,int> hh_thres;
    map<uint32_t,int> hh2_thres,hh3_thres;
    map<uint32_t,int> ld_thres,mc_thres;
    map<uint32_t,int> ss_thres, ws_thres,hk_thres;
    //get real threshod result
    vector<pair<uint32_t,int>> real_threshold_result;
    for(auto it:benchmark_data){
        if(it.second>threshold){
            real_threshold_result.push_back( make_pair(it.first,it.second));
        }
    }
    cout<<"real number of flows exceeding the threshold "<<threshold<<": "<<real_threshold_result.size()<<endl;


    /****************************MCSketch*******************************/
    MC *mcsketch = new MC(total_memory,0.6,1.0);
    mcsketch->Print_basic_info();

    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i =0;i<packet_num;i++){
        mcsketch->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    mcsketch_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);

    mc_thres = mcsketch->Query_threshold(threshold);


    /****************HeavyKeeper*************/
    auto heavykeeper = new HeavyKeeper(total_memory);
    heavykeeper->Print_basic_info();
    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i=0;i<packet_num;i++){
        heavykeeper->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC,&end_time);
    heavykeeper_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    hk_thres = heavykeeper->Query_threshold(threshold);

    /*******************WAVINGSKETCH************/
    auto wavingsketch = new WavingSketch2(total_memory);
    wavingsketch->Print_basic_info();
    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i=0;i<packet_num;i++){
        wavingsketch->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC,&end_time);
    wavingsketch_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    ws_thres = wavingsketch->Query_threshold(threshold);


    /*******************LDSketch*********************/
    LDSketch* ldsketch = new LDSketch(total_memory,100,0.5);
    ldsketch->Print_basic_info();
    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i =0;i<packet_num;i++){
        ldsketch->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    ldsketch_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);

    ld_thres = ldsketch->Query_threshold(threshold);

/**************************1.MVSketch****************************/
    MVSketch* mvsketch = new MVSketch(total_memory);
    mvsketch->Print_basic_info();
    clock_gettime(CLOCK_MONOTONIC,&start_time);
    for(int i =0;i<packet_num;i++){
        mvsketch->Insert(insert_data[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    mvsketch_timediff = (long long)(end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);

    mv_thres = mvsketch->Query_threshold(threshold);



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
    /***************************************MVSketch*************/
    cout <<"******RESULT OF MVSKETCH******"<<endl;
    for(auto it=mv_thres.begin();it!=mv_thres.end();it++){
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
    avg_ab_error = ab_error*1.0/tp;
    avg_re_error = re_error*1.0/tp;
    F1 = 2*precision*recall/(precision+recall);
  //  avg_error = error*1.0 /tp;
    insert_throughput = (double)1000.0*packet_num / mvsketch_timediff;
    cout<<"precision: "<<precision<<endl<<"recall: "<<recall<<endl<<"F1 score: "<<F1<<endl;
    cout<<"average absolute error: "<<avg_ab_error<<endl<<"average relative error: "<<avg_re_error<<endl;
    cout<<"insert throughput: " <<insert_throughput<<endl;
    outFile<<path<<","<<total_memory<<","<<threshold<<",MVSketch,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;


 /*********************************************LDSKETCH**************/
    cout <<"*******RESULT OF LDSketch******"<<endl;
    cnt = 0;
    tp = 0;
    ab_error = 0;
    re_error = 0;
    for(auto it=ld_thres.begin();it!=ld_thres.end();it++){
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
    insert_throughput = (double)1000.0*packet_num/ldsketch_timediff;
    cout<<"precision: "<<precision<<endl<<"recall: "<<recall<<endl<<"F1 score: "<<F1<<endl;
    cout<<"average absolute error: "<<avg_ab_error<<endl<<"average relative error: "<<avg_re_error<<endl;
    cout<<"insert throughput: " <<insert_throughput<<endl;
    outFile<<path<<","<<total_memory<<","<<threshold<<",LDSketch,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;


/************************************WAVINGSKETCH********************/
    cout <<"*******RESULT OF WavingSketch******"<<endl;
    cnt = 0;
    tp = 0;
    ab_error = 0;
    re_error = 0;
    for(auto it=ws_thres.begin();it!=ws_thres.end();it++){
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
    insert_throughput = (double)1000.0*packet_num/wavingsketch_timediff;
    cout<<"precision: "<<precision<<endl<<"recall: "<<recall<<endl<<"F1 score: "<<F1<<endl;
    cout<<"average absolute error: "<<avg_ab_error<<endl<<"average relative error: "<<avg_re_error<<endl;
    cout<<"insert throughput: " <<insert_throughput<<endl;
    outFile<<path<<","<<total_memory<<","<<threshold<<",WavingSketch,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;

/************************************HEAVYKEEPER********************/
    cout <<"*******RESULT OF HeavyKeeper******"<<endl;
    cnt = 0;
    tp = 0;
    ab_error = 0;
    re_error = 0;
    for(auto it=hk_thres.begin();it!=hk_thres.end();it++){
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
    insert_throughput = (double)1000.0*packet_num/heavykeeper_timediff;
    cout<<"precision: "<<precision<<endl<<"recall: "<<recall<<endl<<"F1 score: "<<F1<<endl;
    cout<<"average absolute error: "<<avg_ab_error<<endl<<"average relative error: "<<avg_re_error<<endl;
    cout<<"insert throughput: " <<insert_throughput<<endl;
    outFile<<path<<","<<total_memory<<","<<threshold<<",HeavyKeeper,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;


/************************************MCSketchR********************/
    cout <<"*******RESULT OF MCSketch******"<<endl;
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
    outFile<<path<<","<<total_memory<<","<<threshold<<",MCSketch,"<<insert_throughput<<","<<precision<<","<<recall<<","<<F1<<","<<avg_ab_error<<","<<avg_re_error<<endl;


}


