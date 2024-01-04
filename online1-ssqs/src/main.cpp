#include<bits/stdc++.h>
#include "mmsq_sim2.h"
using namespace std;

int main(int argc, char const *argv[]){
  string filename;
  if(argv[1]==NULL){
    filename = "../IOs-OnlineA/io1/in.txt";
  }else{
    filename = argv[1];
  }
  //filename = "../IOs/io1/in.txt";
  freopen(filename.c_str(),"r",stdin);

  float mean_interarrival, mean_service;
  int num_delays_required, num_of_servers;

  cin>>num_of_servers>>mean_interarrival>>mean_service>>num_delays_required;

  MMSQSim mm1q_sim(num_of_servers, mean_interarrival, mean_service, num_delays_required);
  cout<<"Starting simulation..."<<endl;
  mm1q_sim.run();
  cout<<"Simulation ended."<<endl;

  return 0;
}
