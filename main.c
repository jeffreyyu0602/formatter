#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

int debug = 0;

int readFiles(const char * dirName, char *** filenames) {
  DIR * dir;
  struct dirent * dp;
  FILE * fp;
  int size = 0;

  dir = opendir(dirName);
  if (!dir) {
    fprintf(stderr, "Cannot open directory!\n");
    return -1;
  }

  while (dp = readdir(dir)) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;

    if (size == 0) {
      *filenames = calloc(1, sizeof(void *));
    } else {
      *filenames = realloc(*filenames, (size + 1) * sizeof(void *));
    }
    (*filenames)[size] = malloc(BUFSIZ);
    strcpy((*filenames)[size], dp->d_name);
    size++;
  }
  closedir(dir);
  return size;
}

/*
 * Function name: match
 * Description: This function was a modified version of strcmp. It can
 *              recognize wild card character and store them inside an array
 *              for future use.
 */
int match(char * str, char * pattern, char * charPtr, char * strPtr) {
  /* Debug message. */
  if (debug) {
    fprintf(stderr, "String is %s\n", str);
    fprintf(stderr, "Pattern is %s\n", pattern);
  }

  /* Loop through each character of pattern. */
  while (*pattern) {
    /* If meet a wildcard character, store it to charPtr array and increment
       all pointers. */
    if (*pattern == '.') {
      *charPtr++ = *str++;
      pattern++;
      continue;
    }

    /* If meet an asterisk, loop through the rest of the characters and call
       match recursively. */
    if (*pattern == '*') {
      int result; /* Store the result of recursive call. */
      char * start = str; /* Store the original position. */
      pattern++;
      while (*str) {
        result = match(str, pattern, charPtr, strPtr + (str - start) + 1);
        if (result) {
          strncpy(strPtr, start, str - start);
          break;
        }
        str++;
      }

      /* Debug message. */
      if (debug) {
        fprintf(stderr, "Wild characters are %s\n", charPtr);
        fprintf(stderr, "Wild strings are %s\n", strPtr);
      }
      return result;
    }

    if (*str++ != *pattern++) {
      return 0;
    }
  }

  return 1;
}

void generateNewName(char * filename, char * newname, char * pattern,
                    char * replacement) {
  while (*filename) {
    char wildchar[BUFSIZ] = {0};
    char wildstr[BUFSIZ] = {0};
    char * charPtr = wildchar;
    char * strPtr = wildstr;
    if (match(filename, pattern, wildchar, wildstr)) {
      /* Pattern found. */
      int rpl_idx = 0;
      int wildcard_idx = 0;
      while (replacement[rpl_idx]) {
        if (replacement[rpl_idx] == '.') {
          *newname++ = *charPtr++;
          rpl_idx++;
        } else if (replacement[rpl_idx] == '*') {
          strcpy(newname, strPtr);
          filename += strlen(strPtr) - 1;
          newname += strlen(strPtr);
          strPtr = strchr(strPtr, 0) + 1;
          rpl_idx++;
        } else {
          *newname++ = replacement[rpl_idx++];
        }
      }
      filename += strlen(pattern);
    } else {
      /* Copy one character if pattern not found. */
      *newname++ = *filename++;
    }
  }
}


int main(int argc, char ** argv) {

  char buffer[BUFSIZ];
  char pattern[BUFSIZ];
  char replacement[BUFSIZ];
  int opt;
  int size;
  int test = 0;
  char ** filenames = 0;

  getcwd(buffer, BUFSIZ);
  while ((opt = getopt(argc, argv, "d:xt")) != -1) {
    switch (opt) {
      case 'd':
        strcpy(buffer, optarg);
        break;
      case 'x':
        debug = 1;
        break;
      case 't':
        test = 1;
        break;
      default:
        return EXIT_FAILURE;
    }
  }

  size = readFiles(buffer, &filenames);
  if (size == -1)
    return EXIT_FAILURE;

  while (1) {
    printf("Please enter the pattrern: ");
    if (fgets(pattern, BUFSIZ - 1, stdin) == NULL)
      break;
    printf("Please enter the replace string: ");
    if (fgets(replacement, BUFSIZ - 1, stdin) == NULL)
      break;

    *(strchr(pattern, '\n')) = 0;
    *(strchr(replacement, '\n')) = 0;

    for (int i = 0; i < size; i++) {
      memset(buffer, 0, BUFSIZ);
      generateNewName(filenames[i], buffer, pattern, replacement);
      if (strcmp(filenames[i], buffer)) {
          fprintf(stderr, "%s\n",buffer);
          if (!test)
            rename(filenames[i], buffer);
          strcpy(filenames[i], buffer);
      }
    }
    printf("\n");
  }

  printf("\n");
  if (filenames) {
    for (int i = 0; i < size; i++) {
      free(filenames[i]);
    }
    free(filenames);
  }

  return EXIT_SUCCESS;
}
