#include <vector>
#include <string>
#include <fstream>
#include <map>

struct inst_info {
public:
  int inst;
  int argc;
  char **argv;
};

using InstCache = std::map<int,std::string>;
class RunDLX
{
public:
  RunDLX(const char *ins_path, const char *out_path);
  virtual ~RunDLX();
  int march_ins(std::string instruction);
  int run(const char *ins_line);
  void output(int content);
  struct inst_info * proc_inst_line(const char *ins_line);
  void release_info(struct inst_info * info);
  void cache_inst();
  int runnext();
  const char *find_ins_bynum(int posi);
  int find_num_bymark(const char *mark);
  std::string ToUpcase(std::string& str);
  int* R(const char* reg);
private:
  int reg[32] = {0};
  std::vector<std::string> ins_set = {
    "LW","SW","LB","SB",
    "JR","BEQZ","BNEZ",
    "ADDI","SUBI","ANDI","ORI","XORI",
    "SGEI","SEQI","SNEI","SLTI","SGTI","SLEI",
    "SLLI","SRLI",
    "ADD","SUB","AND","OR","XOR",
    "SEQ","SNE","SLT","SGT","SLE","SGE",
    "HALT",
    "OP"
  };
  std::ifstream inst_file;
  std::ofstream out_file;
  int PC = 0;
  InstCache* instcache;
  InstCache* instmark;
};

//only one class
extern RunDLX *run;
