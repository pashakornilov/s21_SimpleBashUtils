#include "../s21_grep_cat.h"

int main(int argc, char *argv[]) {
  int num_ptrn_str_arr = 0, exit = 0, opt = 0;
  char patterns[BUFFSIZE] = {0};
  struct flags flags = {0};
  char *tmp_patterns[1024] = {NULL};
  const char *short_options = "e:ivclnhsf:o";
  while ((opt = getopt_long(argc, argv, short_options, NULL, NULL)) != -1) {
    switch (opt) {
      case 'e':
        flags.e = 1;
        tmp_patterns[num_ptrn_str_arr++] = optarg;
        break;
      case 'i':
        flags.i = 1;
        break;
      case 'n':
        flags.n = 1;
        break;
      case 'c':
        flags.c = 1;
        break;
      case 'l':
        flags.l = 1;
        break;
      case 'v':
        flags.v = 1;
        break;
      case 's':
        flags.s = 1;
        break;
      case 'h':
        flags.h = 1;
        break;
      case 'o':
        flags.o = 1;
        break;
      case 'f':
        flags.f = reader_flag_f(patterns);
        break;
      default:
        exit = 1;
    }
  }
  if (argc > 2 && exit == 0) {
    if (flags.f != -1) {
      int lenght_patterns = strlen(patterns);
      if (flags.e) {
        if (lenght_patterns > 0 && num_ptrn_str_arr > 0)
          patterns[lenght_patterns++] = '|';
        for (int j = 0; j < num_ptrn_str_arr; ++j) {
          for (int k = 0;
               tmp_patterns[j][k] != '\0' && lenght_patterns < BUFFSIZE; ++k) {
            patterns[lenght_patterns++] = tmp_patterns[j][k];
          }
          if (j + 1 != num_ptrn_str_arr) patterns[lenght_patterns++] = '|';
        }
      }

      if (!flags.e && !flags.f) {
        snprintf(patterns, BUFFSIZE, "%s", argv[optind++]);
      }
      s21_grep(&flags, argv, patterns);
    }
  } else {
    fprintf(stderr, "usage: grep [-eivclnhsfo]\n");
    fprintf(stderr, "\t[-e pattern] [-f file]\n");
    fprintf(stderr, "\t[pattern] [-f file]\n");
  }
  return 0;
}

int reader_flag_f(char *patterns) {
  int res = 1;
  FILE *fp = NULL;
  fp = fopen(optarg, "r");
  if (fp != NULL) {
    int i = 0;
    while (!feof(fp)) {
      char txt_patterns[BUFFSIZE] = {0};
      fgets(txt_patterns, BUFFSIZE, fp);
      for (int j = 0; txt_patterns[j] != '\0' && i < BUFFSIZE; ++j) {
        if (j == 0 && txt_patterns[j] == '\n') patterns[i++] = txt_patterns[j];
        if (txt_patterns[j] != '\n') patterns[i++] = txt_patterns[j];
      }
      if (!feof(fp)) patterns[i++] = '|';
    }
    int lenght_pattern = strlen(patterns);
    if (patterns[lenght_pattern - 1] == '|')
      patterns[lenght_pattern - 1] = '\0';
    fclose(fp);
  } else {
    fprintf(stderr, "grep: %s: No such file or directory\n", optarg);
    res = -1;
  }
  return res;
}

void s21_grep(struct flags *flags, char *argv[], char *patterns) {
  int cflags = REG_EXTENDED, number_line = 1, match_lines = 0, one_more_txt = 0,
      break_point = 0, check_match_file = 0, dont_count = 0, exit = 0,
      exit_while = 0;
  if (argv[optind + 1] != NULL) one_more_txt = 1;
  while (argv[optind] != NULL) {
    int flag_continue = 0;
    FILE *fp = NULL;
    fp = fopen(argv[optind], "r");
    if (fp == NULL) {
      if (!flags->s) {
        fprintf(stderr, "grep: %s: No such file or directory\n", argv[optind]);
      }
      optind++;
      flag_continue = 1;
    }
    if (flag_continue == 0) {
      if (flags->i) cflags = REG_ICASE;
      regfree(&reg);
      regcomp(&reg, patterns, cflags);
      while (!feof(fp)) {
        char buffer[BUFFSIZE] = {0};
        fgets(buffer, BUFFSIZE, fp);
        int count = strlen(buffer);
        if (count > 0) {
          int offset = 0, check_match_lines = 0;
          if (buffer[count - 1] != '\n' && buffer[count] != '\n') {
            if (feof(fp) &&
                ((flags->i && flags->v) || (!flags->i && !flags->v)) &&
                !flags->e && !flags->n && !flags->l && !flags->c && !flags->s &&
                !flags->h && !flags->o) {
            } else {
              buffer[count++] = '\n';
            }
          }
          while ((regexec(&reg, buffer + offset, nmatch, &pmatch, 0) == 0 &&
                  !flags->v && exit_while == 0) ||
                 (regexec(&reg, buffer + offset, nmatch, &pmatch, 0) ==
                      REG_NOMATCH &&
                  flags->v && exit_while == 0)) {
            check_match_lines = check_match_file = 1;
            if (buffer[0] == '\n' && flags->c && !flags->v) dont_count = 1;
            if (one_more_txt > 0 && !flags->c && !flags->l && !flags->s &&
                !flags->h)
              printf("%s:", argv[optind]);
            if (!flags->l && !flags->c && flags->n) printf("%d:", number_line);
            if (!flags->l && !flags->c && !flags->o) printf("%s", buffer);
            if (flags->o) {
              if (flags->v && !flags->l && !flags->c) {
                for (int r = 0; r < count; r++) {
                  printf("%c", buffer[r]);
                }
                exit = 1;
              } else if (!flags->l && !flags->c && exit == 0) {
                break_point = 1;
                for (int r = pmatch.rm_so + offset; r < pmatch.rm_eo + offset;
                     r++) {
                  printf("%c", buffer[r]);
                }
                offset += pmatch.rm_eo;
                if (offset > count) exit = 1;
                if (exit == 0) printf("\n");
              }
            }
            if (break_point == 0) exit_while = 1;
          }
          if (check_match_lines == 1 && dont_count == 0) match_lines++;
          dont_count = exit_while = exit = 0;
          number_line++;
        }
      }
      check_match_file_func(flags, &one_more_txt, &check_match_file, argv);
      check_match_line_func(flags, &one_more_txt, &match_lines, argv);
      fclose(fp);
      number_line = 1;
      match_lines = check_match_file = exit_while = 0;
      optind++;
    }
  }
}

void check_match_file_func(struct flags *flags, const int *one_more_txt,
                           const int *check_match_file, char *argv[]) {
  if (((flags->o || flags->v || flags->h || flags->n) && flags->c &&
       flags->l) ||
      (flags->c && flags->l)) {
    if (*one_more_txt > 0 && !flags->h) {
      printf("%s:%d\n", argv[optind], *check_match_file);
    } else {
      printf("%d\n", *check_match_file);
    }
  }
}

void check_match_line_func(struct flags *flags, const int *one_more_txt,
                           const int *match_lines, char *argv[]) {
  if (flags->l && *match_lines > 0) printf("%s\n", argv[optind]);
  if (flags->c && !flags->l) {
    if (*one_more_txt > 0 && !flags->h) {
      printf("%s:%d\n", argv[optind], *match_lines);
    } else {
      printf("%d\n", *match_lines);
    }
  }
}
