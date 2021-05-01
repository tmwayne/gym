//
// -----------------------------------------------------------------------------
// Quizzer
// -----------------------------------------------------------------------------
//
// Tyler Wayne © 2021
//

#include <stdio.h>             // printf, fopen
#include <stdlib.h>            // srand, rand, RAND_MAX, EXIT_FAILURE
#include <time.h>              // time
#include <limits.h>            // LINE_MAX
#include <string.h>            // strdup
#include <readline/readline.h> // readline
#include "argparse.h"

#include <cstrings.h>         // strmatch
#include <error.h>             // assert
#include "exercise.h"          // exercise_T, exercise_new
#include "registry.h"          // entry_T, entry_new

#define EXERCISE_NAME "quizzer"

#define NLINES 128
#define MAX_BUF 128

struct pair {
  char *a;
  char *b;
};

void shuffle_quiz(struct pair **quiz, int n) {

  srand(time(NULL));

  for(size_t i=n-1; i>0; i--) {
    size_t j = rand() / (RAND_MAX / n);
    struct pair *t = quiz[j];
    quiz[j] = quiz[i];
    quiz[i] = t;
  }

}

int load_quiz(struct pair **quiz, FILE *fd) {

  char line[LINE_MAX+1];
  int n = 0;

  // TODO: prevent segfaults when >2 fields in input
  for ( ; n<NLINES && get_line(line, LINE_MAX, fd)>0; n++ ) {
    if (!(quiz[n] = calloc(1, sizeof(struct pair)))) return -1;
    char *saveptr = NULL;
    quiz[n]->a = strdup(get_tok_r(line, '|', &saveptr));
    quiz[n]->b = strdup(get_tok_r(NULL, '|', &saveptr));
  }

  return n;

}

int give_quiz(struct pair **quiz, int len) {

  char *guess;
  int tries = 0;
  int nright = 0;

  for (int i=0; i<len; i++) {
    printf("%s : ", quiz[i]->a);
    for ( tries=0; tries < 3; tries++) {
      // TODO: use the stack instead of the allocating on the heap every time
      char *guess = readline(NULL);
      if (strcasematch(guess, quiz[i]->b)) {
        nright++;
        break;
      }
      else if (tries < 2) printf("Nope, try again : ");
      else printf("The answer is %s...\n", quiz[i]->b);
      free(guess);
    }
  }

  return nright;

}

double play(int argc, char **argv) {

  // Load command-line arguments
  dict_T configs = dict_new();
  argp_parse(&argp, argc, argv, 0, 0, configs);
  
  // Load quiz
  FILE *fin = fopen(dict_get(configs, "file"), "r");
  struct pair *quiz[NLINES];

  int nlines;
  if ((nlines = load_quiz(quiz, fin)) <= 0) {
    fprintf(stderr, "Failed to allocate memory for quiz...\n");
    exit(EXIT_FAILURE);
  }

  int nlines_config = (long) dict_get(configs, "nlines");
  // Note : nlines_config is a length not an index, so we increment it before
  // setting it as value
  nlines = nlines_config && nlines_config < nlines ? ++nlines_config : nlines;

  // Prepare quiz
  shuffle_quiz(quiz+1, nlines-1); // ignore header

  // Give quiz
  double score = (double) give_quiz(quiz+1, nlines-1) / (nlines-1);

  printf("You scored %.0f%!\n", 100*score);

  // Cleanup
  // TODO: free configs dictionary
  // dict_free(&configs);
  for (int i=0; i<nlines; i++) free(quiz[i]);

  return score;

}

// interface to Gym routine ----------------------------------------------------

exercise_T init() {

  exercise_T exercise = exercise_new();

  exercise->play = play;
  exercise->args = NULL;

  return exercise;

}

void add_to_registry(dict_T registry, char *plugin_path) {

  assert(registry && plugin_path);

  entry_T entry = entry_new(plugin_path, (void *(*)()) init);
  dict_set(registry, EXERCISE_NAME, entry);

}  
