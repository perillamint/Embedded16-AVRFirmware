#ifndef __DEBUGSHELL_HPP
#define __DEBUGSHELL_HPP

#ifndef __cplusplus
#error Debug shell class requires C++.
#endif

#define MAX_CMD_BUF_SIZE 128

class DebugShell
{
private:
  int cursor;
  int read_cmd();
  int eval_cmd();
  int cmd_help();
  int cmd_spidump();
  int cmd_dumpmem();
  int cmd_setmem();
public:
  DebugShell();
  ~DebugShell();
  int init();
  int do_repl();
};

#endif
