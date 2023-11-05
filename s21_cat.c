#include "../s21_grep_cat.h"

int main(int argc, char *argv[]) {
  int exit = 0, opt;
  struct flags flags = {0};
  const char *short_options = "+beEnstTv";
  const struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
      {NULL, 0, NULL, 0}};
  while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'b':
        flags.b = 1;
        break;
      case 'e':
        flags.e = 1;
        flags.v = 1;
        break;
      case 'E':
        flags.e = 1;
        break;
      case 'n':
        flags.n = 1;
        break;
      case 's':
        flags.s = 1;
        break;
      case 't':
        flags.t = 1;
        flags.v = 1;
        break;
      case 'T':
        flags.t = 1;
        break;
      case 'v':
        flags.v = 1;
        break;
      default:
        fprintf(stderr, "cat: illegal option -- %c\n", opt);
        fprintf(stderr, "usage: cat [-benstuv] [file ...]\n");
        exit = 1;
    }
  }
  if (exit != 1) {
    while (argv[optind] != NULL) {
      open_file(flags, argv);
      optind++;
    }
  }
  return 0;
}

void open_file(struct flags flags, char *argv[]) {
  FILE *fp = NULL;
  fp = fopen(argv[optind], "r");
  if (argv[optind] != NULL) {
    if (fp != NULL) {
      s21_cat(&flags, fp);
      fclose(fp);
    } else {
      fprintf(stderr, "cat: %s: No such file or directory\n", argv[optind]);
    }
  }
}

void s21_cat(struct flags *flags, FILE *fp) {
  int c, first_c = 0, current_line, last_line = 0, line_number = 1,
         continue_func = 0;
  while ((c = fgetc(fp)) != EOF) {
    if (flags->s && first_c == 0 && c == 10) {
      current_line = 1;
      if (last_line) {
        continue_func = 1;
      }
      if (continue_func == 0) last_line = current_line;
    } else {
      last_line = 0;
    }
    if (continue_func == 0) {
      if (first_c == 0) {
        if (flags->b && c != '\n') {
          printf("%6d\t", line_number++);
        }
        if (flags->n && !flags->b) {
          printf("%6d\t", line_number++);
        }
        first_c = 1;
      }
      if (flags->e && c == '\n') {
        printf("$");
      }
      if (flags->v && c < 32 && c != 9 && c != 10) {
        printf("^%c", c + 64);
      } else if (flags->v && c == 127) {
        printf("^%c", c - 64);
      } else if (flags->t && c == '\t') {
        printf("^I");
      } else {
        printf("%c", c);
      }
      if (c == '\n') {
        first_c = 0;
      }
    }
    continue_func = 0;
  }
}
