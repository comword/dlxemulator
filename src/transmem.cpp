#include "transmem.h"

#include <fstream>
#include <sstream>
#include <iostream>

TransMem::TransMem()
{
  mem = (char *)malloc(mem_size);
}

TransMem::~TransMem()
{
  free(mem);
}

void TransMem::cp_mem(const char *file_path)
{
  std::ifstream mem_file(file_path);
  std::string line, offset, value;
  unsigned int offset_i, value_i;
  while (std::getline(mem_file, line))
  {
    line = line.substr(0, line.find('\r'));
    std::istringstream ss(line);
    std::stringstream trans;
    if (!getline( ss, offset, ':' )){
      std::cerr<<"Error memory line: "<< line <<std::endl;
      break;
    }
    trans << std::hex << offset;
    trans >> offset_i;
    if(offset_i > mem_size){
      std::cerr<<"Error memory line: "<< line <<std::endl;
      break;
    }
    if (!getline( ss, value, ':' )){
      std::cerr<<"Error memory line: "<< line <<std::endl;
      break;
    }
    std::stringstream trans_v;
    //value = tmp_str + value.substr(2,4);
    trans_v << std::hex << value;
    trans_v >> value_i;
    *(mem + offset_i) = value_i;
  }
}
