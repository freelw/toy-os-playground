#include "mm.h"
#define MAX_L 23
#define SCREEN_LEN 2000
extern char *screen_buff;
extern int cur_l, cur_c;
extern void write_char(char ch, int pos);

void init_screen_buff() {
  cur_l = 0;
  screen_buff = (char *)get_free_page();
  for (int i = 0; i < SCREEN_LEN; ++i) {
    screen_buff[i] = 0;
  }
  for (int i = 80 * 23; i < 80 * 24; ++i) {
    screen_buff[i] = '-';
  }
}

void validate_screen() {
  for (int i = 0; i < SCREEN_LEN; ++i) {
    write_char(screen_buff[i], i);
  }
}

void print_str(char *s) {
  int y = cur_l;
  int x = 0;
  while (s[x] != 0) {
    screen_buff[y * 80 + cur_c] = s[x];
    x++;
    cur_c++;
  }
  validate_screen();
}

void print_crlf() {
  cur_l ++;
  cur_c = 0;
  if (cur_l >= MAX_L) {
    cur_l = MAX_L - 1;
    for (int i = 0; i < cur_l; ++i) {
      for (int j = 0; j < 80; ++j) {
        screen_buff[i * 80 + j] = screen_buff[(i + 1) * 80 + j];
      }
    }
    for (int j = 0; j < 80; ++j) {
      screen_buff[cur_l * 80 + j] = 0;
    }
  }
  validate_screen();
}

void print_num(int num) {
  char buf[64];
  int index = 0;
  for (int i = 0; i < 64; ++i) {
    buf[i] = 0;
  }
  if (num == 0) {
    buf[index++] = '0';
  } else if (num < 0) {
    num = -num;
    buf[index++] = '-';
  }
  char tmp[64];
  int index_tmp = 0;
  while (num > 0) {
    tmp[index_tmp++] = num % 10 + '0';
    num /= 10;
  }
  for (int i = index_tmp - 1; i >= 0; --i) {
    buf[index++] = tmp[i];
  }
  print_str(buf);
}

void print_hex(int num) {
  char buf[64];
  int index = 0;
  for (int i = 0; i < 64; ++i) {
    buf[i] = 0;
  }
  if (num == 0) {
    buf[index++] = '0';
  } else if (num < 0) {
    num = -num;
    buf[index++] = '-';
  }
  buf[index++] = '0';
  buf[index++] = 'x';
  char tmp[64];
  int index_tmp = 0;
  while (num > 0) {
    unsigned char ch = num % 16;
    if (ch < 10) {
      ch += '0';
    } else {
      ch -= 10;
      ch += 'a';
    }
    tmp[index_tmp++] = ch;
    num /= 16;
  }
  for (int i = index_tmp - 1; i >= 0; --i) {
    buf[index++] = tmp[i];
  }
  print_str(buf);
}

void panic(char *msg) {
  print_str(msg);
  while (1) {
    validate_screen();
  }
}

void print_shell_line(char *buffer) {
  int start = 24 * 80;
  screen_buff[start++] = 's';
  screen_buff[start++] = 'h';
  screen_buff[start++] = 'e';
  screen_buff[start++] = 'l';
  screen_buff[start++] = 'l';
  screen_buff[start++] = '>';
  for (int i = start; i < 25 * 80; ++i) {
    screen_buff[i] = buffer[i - start];
  }
  validate_screen();
}
