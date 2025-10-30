#include "sched.h"

typedef int (*sys_call)();
extern void print_str(char *s);
extern void print_num(int num);
extern void print_crlf();
extern void get_codes(char *buffer);
extern void clean_keyboard_buffer();
extern void print_shell_line(char *buffer);

unsigned char get_fs_byte(const char *addr) {
  unsigned register char _v;
  __asm__("movb %%fs:%1,%0" : "=r"(_v) : "m"(*addr));
  return _v;
}

extern inline void put_fs_byte(char val, char *addr) {
  __asm__("movb %0,%%fs:%1" ::"r"(val), "m"(*addr));
}

int _test_sys_call() {
  print_str("ggg");
  return 0;
}

int _test_sys_call1() { return 0; }

int _test_sys_call2() {
  print_str("hhh");
  return 0;
}

int _sys_print_str(char *msg) {
  char buf[64];
  for (int j = 0; j < 64; ++j) {
    buf[j] = 0;
  }
  int i = 0;
  while (1) {
    buf[i] = get_fs_byte(msg + i);
    if (buf[i] == 0) {
      break;
    }
    i++;
  }
  print_str(buf);
  return 0;
}

int equal_str(char *file_name, char *name) {
  for (int i = 0; i < 8; ++i) {
    if (file_name[i] != name[i]) {
      return 1;
    } else if (file_name[i] == 0 && name[i] == 0) {
      return 0;
    }
  }
  return 0;
}

int get_file_index_by_name(char *file_name) {
  int *p_cnt = 0x6dd84;
  char *meta_start = 0x6dd84 + 4;
  int file_cnt = *p_cnt;
  for (int i = 0; i < file_cnt; ++i) {
    int *p_length = meta_start + 12 * i;
    char *name = meta_start + 12 * i + 4;
    if (0 == equal_str(file_name, name)) {
      return i;
    }
  }
  return -1;
}

char *get_file_buffer(int index, unsigned long *plength) {
  char *file_buffer_start = 0x6de00;
  char *meta_start = 0x6dd84 + 4;
  *plength = *((unsigned long *)(meta_start + 12 * index));
  return file_buffer_start + 20 * 1024 * index;
}

void bread(unsigned long page, int FourKnum, int fs_index) {
  unsigned long length = 0;
  char *f_start = get_file_buffer(fs_index, &length);
  for (int i = 0; i < 4096; ++i) {
    ((char *)page)[i] = (char *)(f_start + FourKnum * 4096)[i];
  }
}

int _sys_read_file_content(char *_u_file_name, char *_u_buffer) {
  char file_name[64];
  for (int i = 0; i < 64; ++i) {
    file_name[i] = 0;
  }
  for (int i = 0; i < 8; ++i) {
    file_name[i] = get_fs_byte(_u_file_name + i);
  }
  int index = get_file_index_by_name(file_name);
  if (index >= 0) {
    unsigned long length = 0;
    char *start = get_file_buffer(index, &length);
    for (int i = 0; i < length; ++i) {
      put_fs_byte(start[i], _u_buffer + i);
    }
  }
  return 0;
}

int _sys_print_num(int num) {
  print_num(num);
  return 0;
}

int _sys_print_crlf() {
  print_crlf();
  return 0;
}

int _sys_exec(char *_u_file_name, unsigned long eip_pos) {
  char file_name[64];
  for (int i = 0; i < 64; ++i) {
    file_name[i] = 0;
  }
  for (int i = 0; i < 8; ++i) {
    file_name[i] = get_fs_byte(_u_file_name + i);
  }
  unsigned long data_limit = get_limit(0x17);
  unsigned long data_base = get_data_base(current);
  int index = get_file_index_by_name(file_name);
  if (index >= 0) {
    current->fs_index = index;
    get_file_buffer(index, &(current->end_data));
  } else {
    return -1;
  }
  free_page_tables(data_base, data_limit);
  *((unsigned long *)(eip_pos)) =
      0;  // 我们的应用程序的main从0x0开始
          // 注意，应用程序的main一定要是第一个函数！！！
  *((unsigned long *)(eip_pos + 12)) = data_limit;  // esp 顶死
  return 0;
}

void _sys_get_keyboard_code_buffer(char *_u_buffer) {
  char buffer[256];
  get_codes(buffer);
  for (int i = 0; i < 256; ++i) {
    put_fs_byte(buffer[i], _u_buffer + i);
  }
}

void _sys_clean_keyboard_code_buffer() { clean_keyboard_buffer(); }

void _sys_print_shell_line(char *shell_content) {
  char buffer[256];
  for (int i = 0; i < 256; ++i) {
    buffer[i] = get_fs_byte(shell_content + i);
  }
  print_shell_line(buffer);
}

void _sys_list_files(char *_u_buffer) {
  char *meta_start_with_cnt = 0x6dd84;
  char buffer[124];
  for (int i = 0; i < 124; ++i) {
    put_fs_byte(meta_start_with_cnt[i], _u_buffer + i);
  }
}

extern void sys_fork();
extern void sys_print_str();
extern void sys_print_num();
extern void sys_read_file_content();
extern void sys_exec();
extern void sys_get_keyboard_code_buffer();
extern void sys_clean_keyboard_code_buffer();
extern void sys_print_shell_line();
extern void sys_list_files();
extern void sys_exit();
extern void sys_print_crlf();

extern sys_call sys_call_table[] = {
    _test_sys_call,
    sys_fork,
    _test_sys_call1,
    _test_sys_call2,
    sys_print_str,
    sys_print_num,
    sys_read_file_content,
    sys_exec,
    sys_get_keyboard_code_buffer,
    sys_clean_keyboard_code_buffer,
    sys_print_shell_line,
    sys_list_files,
    sys_exit,
    sys_print_crlf,
};