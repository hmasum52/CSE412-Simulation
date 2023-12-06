#include<bits/stdc++.h>
#include "mm1q_sim.h"
using namespace std;

int main(int argc, char const *argv[]){
  string filename;
  if(argv[1]==NULL){
    filename = "in.txt";
  }else{
    filename = argv[1];
  }
  //filename = "../IOs/io1/in.txt";
  freopen(filename.c_str(),"r",stdin);

  float mean_interarrival, mean_service;
  int num_delays_required;

  cin>>mean_interarrival>>mean_service>>num_delays_required;

  MM1QSim mm1q_sim(mean_interarrival, mean_service, num_delays_required);
  cout<<"Starting simulation..."<<endl;
  mm1q_sim.run();
  cout<<"Simulation ended."<<endl;

  return 0;
}
