#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_COUNT 15000
#define MAX_SUCCESSOR_COUNT MAX_WORD_COUNT / 2

char book[] = {
#embed "pg84.txt"
    , '\0'}; /// Stores the content of the file as an array of chars. Makes
             /// `book` a string.

/// Array of tokens registered so far.
/// No duplicates are allowed.
char *tokens[MAX_WORD_COUNT];
/// `tokens`'s current size
size_t tokens_size = 0;

/// Array of successor tokens
/// One token can have many successor tokens. `succs[x]` corresponds to
/// `token[x]`'s successors.
/// We store directly tokens instead of token_ids, because we will directly
/// print them. If we wanted to delete the book, then it would make more sense
/// to store `token_id`s
char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
/// `succs`'s current size
size_t succs_sizes[MAX_WORD_COUNT];

/// Overwrites non-printable characters in `book` with a space.
/// Non-printable characters may lead to duplicates like
/// `"\xefthe" and "the"` even both print `the`.
void replace_non_printable_chars_with_space() {
  for (size_t i = 0; i < strlen(book); ++i) {
    if (!isprint(book[i])) {
      book[i] = ' ';
    }
  }
}

/// Returns the id (index) of the token, creating it if necessary.
///
/// Returns token id if token exists in \c tokens, otherwise creates a new entry
/// in \c tokens and returns its token id.
///
/// \param token token to look up (or insert)
/// \return Index of `token` in \c tokens array
size_t token_id(char *token) {
  size_t id;
  for (id = 0; id < tokens_size; ++id) {
    if (strcmp(tokens[id], token) == 0) {
      return id;
    }
  }
  tokens[id] = token;
  ++tokens_size;
  return id;
}

/// Appends the token \c succ to the successors list of \c token.
void append_to_succs(char *token, char *succ) {
  auto next_empty_index_ptr = &succs_sizes[token_id(token)];

  if (*next_empty_index_ptr >= MAX_SUCCESSOR_COUNT) {
    printf("Successor array full.");
    exit(EXIT_FAILURE);
  }

  succs[token_id(token)][(*next_empty_index_ptr)++] = succ;
}

/// Creates tokens on \c book and fills \c tokens and \c succs using
/// the functions \c token_id and \c append_to_succs.
void tokenize_and_fill_succs(char *delimiters, char *str) {
  char *current_word = strtok(str, delimiters);
  while (true) {
    char *next_word = strtok(NULL, delimiters);
    token_id(current_word);
    if (next_word != NULL) {
      token_id(next_word);
      append_to_succs(current_word, next_word);
      current_word = next_word;
    }

    else {
      break;
    }
  }
}

/// Returns last character of a string
char last_char(char *str) {
  size_t length = strlen(str);
  char last_char = str[length - 1];
  return last_char;
}

/// Returns whether the token ends with `!`, `?` or `.`.
bool token_ends_a_sentence(char *token) {
  size_t length = strlen(token);
  char chr = last_char(token);
  if (chr == '!' || chr == '?' || chr == '.') {
    return true;
  }

  else {
    return false;
  }
}

/// Returns a random `token_id` that corresponds to a `token` that starts with a
/// capital letter.
/// Uses \c tokens and \c tokens_size.
size_t random_token_id_that_starts_a_sentence() {
  size_t count = 0;
  char tokens_with_capital_letter[tokens_size];
  for (size_t i = 0; i < tokens_size; ++i) {
    if (isupper(tokens[i][0])) {
      tokens_with_capital_letter[count++] = i;
    }
  }
  return tokens_with_capital_letter[rand() % count];
}

/// Generates a random sentence using \c tokens, \c succs, and \c succs_sizes.
/// The sentence array will be filled up to \c sentence_size-1 characters using
/// random tokens until:
/// - a token is found where \c token_ends_a_sentence
/// - or more tokens cannot be concatenated to the \c sentence anymore.
/// Returns the filled sentence array.
///
/// @param sentence array what will be used for the sentence.
/// @param sentence_size Will be overwritten. Does not have to be initialized.
/// @return input sentence pointer
char *generate_sentence(char *sentence, size_t sentence_size) {
  size_t current_token_id = random_token_id_that_starts_a_sentence();
  auto token = tokens[current_token_id];

  sentence[0] = '\0';
  strcat(sentence, token);
  if (token_ends_a_sentence(token))
    return sentence;

  // Calculated sentence length for the next iteration.
  // Used to stop the loop if the length exceeds sentence size
  size_t sentence_len_next;
  // Concatenates random successors to the sentence as long as
  // `sentence` can hold them.
  do {
    size_t succ_number = succs_sizes[current_token_id];
    if (succ_number == 0) {
      break;
    }
    char *next_word = succs[current_token_id][rand() % succ_number];
    size_t length_of_next_word = strlen(next_word);
    sentence_len_next = strlen(sentence) + length_of_next_word;

    if (sentence_len_next < sentence_size - 1) {
      break;
    }

    strcat(sentence, " ");
    strcat(sentence, next_word);

    if (token_ends_a_sentence(next_word)) {
      break;
    }

    current_token_id = token_id(next_word);
  } while (sentence_len_next < sentence_size - 1);
  return sentence;
}

int main() {
  replace_non_printable_chars_with_space();

  char *delimiters = " \n\r";
  tokenize_and_fill_succs(delimiters, book);

  char sentence[1000];
  srand(time(nullptr)); // Be random each time we run the program

  // Generate sentences until we find a question sentence.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '?');
  puts(sentence);
  puts("");

  // Initialize `sentence` and then generate sentences until we find a sentence
  // ending with an exclamation mark.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '!');
  puts(sentence);
}