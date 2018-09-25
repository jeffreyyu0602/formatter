#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

/* Function name: readFiles
 * Description: this function reads all the filenames from a directory.
 * Parameters: dirName: the path of the directory.
 *             filenames: a pointer to an array of string.
 * Return value: number of filenames read in.
 */
int readFiles(const char * dirName, char *** filenames) {
  DIR * dir; /* A pointer to the directory that we are reading files from. */
  struct dirent * dp; /* A pointer to each file of in the directory. */
  int size = 0; /* Number of files that we read. */

  /* Open directory and check if the process succeeded. */
  dir = opendir(dirName);
  if (!dir) {
    fprintf(stderr, "Cannot open directory!\n");
    return -1;
  }

  /* Read in each file. */
  while (dp = readdir(dir)) {
    /* Eliminate current directory and parrent directory. */
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;

    /* Allocate space for the array. */
    if (size == 0) {
      *filenames = calloc(1, sizeof(void *));
    } else {
      *filenames = realloc(*filenames, (size + 1) * sizeof(void *));
    }

    /* Allocate memory for filename and copy the name. */
    (*filenames)[size] = malloc(BUFSIZ);
    strcpy((*filenames)[size], dp->d_name);
    size++;
  }
  closedir(dir);
  return size;
}

/*
 * Function name: match
 * Description: This function was a improved version of strcmp. It can
 *              recognize wild card characters and store replacement inside 
 *              arrays for future use.
 * Parameters: str: a pointer to the string that we want to check.
 *             pattern: a pointer to the pattern string.
 *             charPtr: a pointer to the char array that stores replacement
 *                      for wildcard dot.
 *             strPtr: a pointer to a char array that stores the replacement
 *                     for wildcard Kleene star.
 * Return value: 1 indicates that pattern is found.
 */
int match(char * str, char * pattern, char * charPtr, char * strPtr) {
  /* Loop through each character of pattern. */
  while (*pattern) {
    /* If meet a wildcard dot, store it to charPtr array and increment
       str, pattern, and charPtr pointers. */
    if (*pattern == '.') {
      *charPtr++ = *str++;
      pattern++;
      continue;
    }

    /* If meet a Kleene star, loop through the rest of the characters and call
       match recursively until the pattern is found or we reach the end of the
       string. Store the replacement string then. */
    if (*pattern == '*') {
      char * start = str; /* Store the original position. */
      pattern++;
      while (*str) {
        if (match(str, pattern, charPtr, strPtr + (str - start) + 1)) {
          strncpy(strPtr, start, str - start);
          return 1;
        }
        str++;
      }
      return 0;
    }

    /* Return 0 if meet an unmatched character. */
    if (*str++ != *pattern++) {
      return 0;
    }
  }
  return 1;
}

/* Function name: generateNewName
 * Description: This function generates a new name based on the pattern found
 *              in the old filename and replacement.
 * Parameters: filename: a pointer to the filename string.
 *             newname: a pointer to a empty char array which stores the new
 *                      name.
 *             pattern: a pointer to the pattern string.
 *             replacement: a pointer to the replacement string.
 * Return value: none.
 */
void generateNewName(char * filename, char * newname, char * pattern,
                     char * replacement) {
  char * charPtr; /* A pointer that points to wildcard buffer. */
  char * strPtr; /* A pointer that points to wildstr buffer. */
  char * rplPtr; /* A pointer to the replacement. */
  char wildchar[BUFSIZ]; /* A buffer that stores all the replacement for
                            wildcard dot(.). */
  char wildstr[BUFSIZ]; /* A buffer that stores all the replacement for wildcard
                           Kleene star(*). */

  while (*filename) {
    /* Fill array with 0 for convenience. */
    memset(wildchar, 0, BUFSIZ);
    memset(wildstr, 0, BUFSIZ);
    if (match(filename, pattern, wildchar, wildstr)) {
      /* Reset pointers to the begining of corresponding arrays. */
      charPtr = wildchar;
      strPtr = wildstr;
      rplPtr = replacement;

      /* Loop through each character of replacement and substitute wildcard
         with letters in original filename. */
      while (*rplPtr) {
        if (*rplPtr == '.') {
          *newname++ = *charPtr++;
          rplPtr++;
        } else if (*rplPtr == '*') {
          strcpy(newname, strPtr);
          filename += strlen(strPtr) - 1;
          newname += strlen(strPtr);
          strPtr = strchr(strPtr, 0) + 1;
          rplPtr++;
        } else {
          *newname++ = *rplPtr++;
        }
      }
      filename += strlen(pattern);
    } else {
      /* Copy one character if pattern not found. */
      *newname++ = *filename++;
    }
  }
}

/*
 * Function name: main
 * Description: This is the main driver of this program. It parse command line
 *              argument from the user and prompt the user to enter pattern and
 *              replacement. Then it called generateNewName to get the new name
 *              of each file in a loop.
 * Parameters: argc: the number of argument.
 *             argv: the actual argument stored in an array of char pointer.
 * Return value: 0 on success.
 */
int main(int argc, char ** argv) {
  char buffer[BUFSIZ]; /* Used to store path and new name. */
  char pattern[BUFSIZ]; /* Store patterns string that the user enters. */
  char replacement[BUFSIZ]; /* Store replacement string that the user enters. */
  int opt; /* Command line argument. */
  int size; /* Number of files in a directory. */
  static int debug = 0; /* Debug mode. */
  char ** filenames = 0; /* An array of string that stores all the filenames. */

  getcwd(buffer, BUFSIZ); /* Default current directory. */

  /* Parse command line arguments. */
  while ((opt = getopt(argc, argv, "d:x")) != -1) {
    switch (opt) {
      case 'd':
        strcpy(buffer, optarg);
        break;
      case 'x':
        debug = 1;
        break;
      case '?':
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
          if (!debug)
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
