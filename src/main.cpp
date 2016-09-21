#include <sys/types.h>
#include <sys/stat.h>
#include <functional>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>
#include <string.h>
#include <map>

#include "transmem.h"
#include "run.h"

TransMem *memory;
RunDLX *run;

void exit_handler(int s);

namespace {

  struct arg_handler {
    typedef std::function<int(int, const char **)> handler_method;

    const char *flag;
    const char *param_documentation;
    const char *documentation;
    const char *help_group;
    handler_method handler;
  };

  void printHelpMessage(const arg_handler *arguments, size_t num_arguments);
}  // namespace

int main(int argc, char *argv[])
{
  const char *ins_file = "undefined";
  const char *mem_file = "undefined";
  const char *out_file = "undefined";
  const arg_handler arg_proc[]={
    {
      "-i","<instruction filename>","",
      nullptr,
      [&ins_file](int num_args, const char **params) -> int {
        if(num_args < 1) return -1;
        ins_file = params[0];
        return 1;
      }
    },
    {
      "-m","<memory filename>","",
      nullptr,
      [&mem_file](int num_args, const char **params) -> int {
        if(num_args < 1) return -1;
        mem_file = params[0];
        return 1;
      }
    },
    {
      "-o","<output filename>","",
      nullptr,
      [&out_file](int num_args, const char **params) -> int {
        if(num_args < 1) return -1;
        out_file = params[0];
        return 1;
      }
    }
  };
  const size_t num_arguments =
  sizeof(arg_proc) / sizeof(arg_proc[0]);
  --argc;
  ++argv;
  while (argc) {
    if(!strcmp(argv[0], "--help")) {
      printHelpMessage(arg_proc, num_arguments);
      return 0;
    } else {
      bool arg_handled = false;
      for (size_t i = 0; i < num_arguments; ++i) {
        auto &arg_handler = arg_proc[i];
        if (!strcmp(argv[0], arg_handler.flag)) {
          argc--;
          argv++;
          int args_consumed = arg_handler.handler(argc, (const char **)argv);
          if (args_consumed < 0) {
            printf("Failed parsing parameter '%s'\n", *(argv - 1));
            exit(1);
          }
          argc -= args_consumed;
          argv += args_consumed;
          arg_handled = true;
          break;
        }
      }
      // Skip other options.
      if (!arg_handled) {
        --argc;
        ++argv;
      }
    }
  }
  //INIT
  if(strcmp(ins_file, "undefined")==0 && strcmp(out_file, "undefined")==0) {
	fprintf(stderr,"Instruction filename or output filename undefined. With --help to view available command line paramters.\n");
    std::exit(0);
  }
  run = new RunDLX(ins_file, out_file);
  memory = new TransMem();
  if(strcmp(mem_file, "undefined")!=0){
    memory->cp_mem(mem_file);
  }
  run->cache_inst();
  while(run->runnext());
  delete(memory);
  delete run;
}

namespace {
  void printHelpMessage(const arg_handler *arguments,size_t num_arguments)
  {
    // Group all arguments by help_group.
    std::multimap<std::string, const arg_handler *> help_map;
    for (size_t i = 0; i < num_arguments; ++i) {
      std::string help_group;
      if( arguments[i].help_group ) {
        help_group = arguments[i].help_group;
      }
      help_map.insert( std::make_pair(help_group, &arguments[i]) );
    }
    printf("Command line paramters:\n");
    std::string current_help_group;
    auto it = help_map.begin();
    auto it_end = help_map.end();
    for (; it != it_end; ++it) {
      if (it->first != current_help_group) {
        current_help_group = it->first;
        printf("\n%s\n", current_help_group.c_str());
      }
      const arg_handler *handler = it->second;
      printf("%s", handler->flag);
      if (handler->param_documentation) {
        printf(" %s", handler->param_documentation);
      }
      printf("\n");
      if (handler->documentation) {
        printf("\t%s\n", handler->documentation);
      }
    }
  }
}  // namespace
void exit_handler(int n)
{
  if (n == 2){
//    std::system("clear");
    int status = 0;
    std::exit(status);
  }
  else
  std::exit(n);
}
