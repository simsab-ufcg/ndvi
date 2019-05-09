#include "parse_args.h"

/**
 * Convert array of pointers to vector of strings
 * */
vector<string> convertToVector(int argc, char * argv[]){
  vector<string> args;
  for(int i = 1; i < argc; i++) args.push_back(argv[i]);
  return args;
}

/**
 * Find if there is specific flag, if there is, remove it from array
 * */
bool hasFlag(vector<string> & args, string flag){
  int size = args.size();
  for(int i = 0; i < size; i++){
    if(args[i] == flag){
      for(int j = i + 1; j < size; j++) args[j - 1] = args[j];
      args.resize(size - 1);
      return true;
    }
  }
  return false;
}