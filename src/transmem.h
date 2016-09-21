#include <string>

class TransMem
{
public:
  TransMem();
  virtual ~TransMem();
  void cp_mem(const char *file_path);
  char * mem;
private:
  unsigned int mem_size = 131072;
};

//only one class
extern TransMem *memory;
