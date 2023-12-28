/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "elf.h"
#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"

#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_print_user_backtrace:
      return print_user_backtrace(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}


// added @lab1_challenge1

extern elf_ctx elf_loader;

ssize_t print_user_backtrace(int64 depth){
  // uint64 user_sp_top = current->trapframe->regs.sp + 24;
  // // 目前的调用栈深度
  // int loc_depth = 0;
  //// for(uint64 i = user_sp_top;loc_depth<depth;i+=16){

  // }

  uint64 user_sp = current->trapframe->regs.sp + 16 + 8;
  int64 cur_depth = 0;
  for (uint64 cur_p = user_sp; cur_depth <= depth; cur_p += 16) {
    uint64 ra = *(uint64 *) cur_p;
    if (ra == 0) break;// * 到达用户栈底
    // * 追踪符号
    uint64 tmp = 0;
    int symbol_idx = -1;
    // sprint("ra: %x\n", ra);
    for (int i = 0; i < elf_loader.symbol_cnt; i++) {
      if (elf_loader.symbols[i].st_info == STT_FUNC && elf_loader.symbols[i].st_value < ra && elf_loader.symbols[i].st_value > tmp) {
        tmp = elf_loader.symbols[i].st_value;
        symbol_idx = i;
      }
    }
    //sprint("Symbol_idx: %d %x depth=%d\n", symbol_idx, elf_loader.symbols[symbol_idx].st_value,depth);
    if (symbol_idx != -1) {
      if (elf_loader.symbols[symbol_idx].st_value >= 0x81000000 )
        sprint("%s\n", &elf_loader.str_table[elf_loader.symbols[symbol_idx].st_name]);
        // && elf_loader.symbols[symbol_idx].st_value <= 0x81000000  + 90 * (depth - 1)
      else
        continue;
    } else {
      sprint("failed to backtrace symbol %lx\n", ra);
    }
    cur_depth++;
  }
  return 0;
}