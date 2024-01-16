// author: Hasan Masum(1805052)

#include<bits/stdc++.h>
#include "inventory.h"
using namespace std;

int main(int argc, char const *argv[]){
  string filename;
  if(argv[1]==NULL){
    filename = "../IOs/io3/in.txt";
  }else{
    filename = argv[1];
  }

  Inventory inventory(filename, "out.txt");
  inventory.run();

  return 0;
}
