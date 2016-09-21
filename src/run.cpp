#include "run.h"
#include "transmem.h"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <string.h>

#define C(x) (*(tmp_inst->argv+(x)))
#define M (size_t)(memory->mem)

RunDLX::RunDLX(const char *ins_path, const char *out_path) :
inst_file(ins_path),
out_file(out_path, std::ios::out),
instcache(new InstCache),
instmark(new InstCache)
{
}

RunDLX::~RunDLX()
{
  delete instmark;
  delete instcache;
  inst_file.close();
  out_file.close();
}

struct inst_info * RunDLX::proc_inst_line(const char *ins_line)
{
  std::istringstream ss(ins_line);
  std::string instruction;
  if (!getline( ss, instruction, ' ' )){
    std::cerr<<"Error instruction line: "<< ins_line <<std::endl;
    return (struct inst_info *)-1;
  }
  struct inst_info *tmp = new struct inst_info;
  int ret = march_ins(instruction);
  if (ret > 40){
    std::cerr<<"Error instruction line: "<< ins_line <<std::endl;
    return (struct inst_info *)-2;
  }
  (*tmp).inst = ret;
  std::string m_tmp(ins_line);
  (*tmp).argc = std::count(m_tmp.begin(), m_tmp.end(), ' ');
  if ((*tmp).argc != 0){
    (*tmp).argv = (char **)malloc(((*tmp).argc)*sizeof(char *));
    for(int i=0; i<(*tmp).argc;i++){
      *((*tmp).argv+i) = (char *)malloc(15*sizeof(char));
      //memset((*tmp).argv+i,0,15*sizeof(char));
      std::string tmp_s;
      if (!getline( ss, tmp_s, ' ')){
        std::cerr<<"Error instruction line: "<< ins_line <<std::endl;
        //return (struct inst_info *)-1;
      }
      strcpy(*((*tmp).argv+i),tmp_s.c_str());
    }
  }
  return tmp;
}

int* RunDLX::R(const char* reg_str)
{
  if (*reg_str == 'R')
  return reg+atoi(reg_str+1);
  else
  return (int*)0;
}

const char * RunDLX::find_ins_bynum(int posi)
{
  for (InstCache::iterator i=instcache->begin(); i!=instcache->end();i++){
    if(i->first == posi )
    return i->second.c_str();
  }
  return (const char *)0;
}

int RunDLX::runnext()
{
  run(find_ins_bynum(PC));
  PC++;
  return 1;
}

int RunDLX::find_num_bymark(const char *mark)
{
  for (InstCache::iterator i=instmark->begin(); i!=instmark->end();i++){
    if(i->second == std::string(mark) )
    return i->first;
  }
  return -1;
}

void RunDLX::cache_inst()
{
  int i = 0;
  std::string line;
  while (std::getline(inst_file, line))
  {
    line = ToUpcase(line);
    line.erase(std::remove(line.begin(),line.end(),'\t'),line.end());
    if(std::count(line.begin(), line.end(), ';') != 0)
      line = line.substr(0, line.find(";"));
    if(std::count(line.begin(), line.end(), ':') != 0){ //has mark
      std::string tmp;
      std::istringstream ss(line);
      if (!getline( ss, tmp, ':')){
        std::cerr<<"Error instruction line: "<< line <<std::endl;
        continue;
      }
      instmark->insert(std::pair<int,std::string>(i,tmp));
      if (!getline( ss, tmp, ':')){
        std::cerr<<"Error instruction line: "<< line <<std::endl;
        continue;
      }
      if(*tmp.c_str()==' ')
        tmp.erase(tmp.begin()+(int)tmp.find(" "));

      instcache->insert(std::pair<int,std::string>(i,tmp));
      i++;
    } else {
      instcache->insert(std::pair<int,std::string>(i,line));
      i++;
    }
  }
}

void RunDLX::release_info(struct inst_info * info)
{
  if(info->argc != 0){
    for (int i=0;i<info->argc;i++)
    delete *(info->argv + i);
    delete info->argv;
  }
  delete info;
}

int RunDLX::march_ins(std::string instruction)
{
  auto it = std::find(ins_set.begin(), ins_set.end(), instruction);
  if (it == ins_set.end()){
    return -1;
  } else {
    return std::distance(ins_set.begin(), it);
  }
}

void RunDLX::output(int content)
{
  char tmp[12]={0};
  //sprintf(tmp,"%hhd",content);
  sprintf(tmp,"%d",content);
  std::cout<<tmp<<std::endl;
  out_file<<tmp<<std::endl;
}

std::string RunDLX::ToUpcase(std::string& str)
{
  std::locale settings;
  std::string converted;

  for(size_t i = 0; i < str.size(); ++i)
  converted += (std::toupper(str[i], settings));

  return converted;
}

int RunDLX::run(const char * ins_line)
{
  //std::cout<<ins_line<<std::endl;
  struct inst_info *tmp_inst = proc_inst_line(ins_line);
  if((size_t)tmp_inst == (size_t)-1 || (size_t)tmp_inst == (size_t)-2)
  return -1;
  int tmp;
  switch(tmp_inst->inst){
    case 0://LW
    *R(C(0)) =
    (*(char*)(atoi(C(2)) + *R(C(1)) + M) << 24)    |
    (*(char*)(atoi(C(2)) + *R(C(1)) + M + 1) << 16)|
    (*(char*)(atoi(C(2)) + *R(C(1)) + M + 2) << 8) |
    (*(char*)(atoi(C(2)) + *R(C(1)) + M + 3));
    break;
    case 1://SW
    *(char*)(atoi(C(2)) + *R(C(1)) + M) = *R(C(0))&(0xff<<24);
    *(char*)(atoi(C(2)) + *R(C(1)) + M + 1) = *R(C(0))&(0xff<<16);
    *(char*)(atoi(C(2)) + *R(C(1)) + M + 2) = *R(C(0))&(0xff<<8);
    *(char*)(atoi(C(2)) + *R(C(1)) + M + 3) = *R(C(0))&(0xff);
    break;
    case 2://LB
    *R(C(0)) = *(char*)(atoi(C(2)) + *R(C(1)) + M)&0xff;
    break;
    case 3://SB
    *(char*)(atoi(C(2)) + *R(C(1)) + M) = *R(C(0))&0xff;
    break;
    case 4://JR
    tmp = find_num_bymark(C(0));
    if (tmp == -1)
    PC = *R(C(0));
    else
    PC = tmp - 1;
    break;
    case 5://BEQZ
    if(*R(C(0)) == 0){
      tmp = find_num_bymark(C(1));
      if (tmp == -1)
      PC = PC + atoi(C(1)) - 1;
      else
      PC = tmp - 1;
    }
    break;
    case 6://BNEZ
    if(*R(C(0)) != 0){
      tmp = find_num_bymark(C(1));
      if (tmp == -1)
      PC = PC + atoi(C(1)) - 1;
      else
      PC = tmp - 1;
    }
    break;
    case 7://ADDI
    *R(C(0)) = (*R(C(1)) + atoi(C(2)));
    break;
    case 8://SUBI
    *R(C(0)) = (*R(C(1)) - atoi(C(2)));
    break;
    case 9://ANDI
    *R(C(0)) = (*R(C(1)) & atoi(C(2)));
    break;
    case 10://ORI
    *R(C(0)) = (*R(C(1)) | atoi(C(2)));
    break;
    case 11://XORI
    *R(C(0)) = (*R(C(1)) ^ atoi(C(2)));
    break;
    case 12://SGEI
    if(*R(C(1))>=atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 13://SEQI
    if(*R(C(1))==atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 14://SNEI
    if(*R(C(1))!=atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 15://SLTI
    if(*R(C(1))<atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 16://SGTI
    if(*R(C(1))>atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 17://SLEI
    if(*R(C(1))<=atoi(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 18://SLLI
    *R(C(0)) = *R(C(1)) << atoi(C(2));
    break;
    case 19://SRLI
    *R(C(0)) = *R(C(1)) >> atoi(C(2));
    break;
    case 20://ADD
    *R(C(0)) = (*R(C(1)) + *R(C(2)));
    break;
    case 21://SUB
    *R(C(0)) = (*R(C(1)) - *R(C(2)));
    break;
    case 22://AND
    *R(C(0)) = (*R(C(1)) & *R(C(2)));
    break;
    case 23://OR
    *R(C(0)) = (*R(C(1)) | *R(C(2)));
    break;
    case 24://XOR
    *R(C(0)) = (*R(C(1)) ^ *R(C(2)));
    break;
    case 25://SEQ
    if(*R(C(1))==*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 26://SNE
    if(*R(C(1))!=*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 27://SLT
    if(*R(C(1))<*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 28://SGT
    if(*R(C(1))>*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 29://SLE
    if(*R(C(1))<=*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 30://SGE
    if(*R(C(1))>=*R(C(2)))
    *R(C(0)) = 1;
    else
    *R(C(0)) = 0;
    break;
    case 31://HALT
    std::cout<<"CPU halted."<<std::endl;
    std::exit(0);
    break;
    case 32://OP
    if(tmp_inst->argc == 1)
    output(*R(C(0)));
    if(tmp_inst->argc == 2)
    output(*R(C(0)));
    break;
    break;
    case 33:
    break;
    case 34:
    break;
    case 35:
    break;
    case 36:
    break;
    case 37:
    break;
    case 38:
    break;
    case 39:
    break;
  }
  reg[0]=0;
  release_info(tmp_inst);
  return 1;
}
