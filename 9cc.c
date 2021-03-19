#include <ctype.h>
#include <stdarg.h>
/* C99 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
};

Token *token;

/* Output error. This functions is presented in K&R C */
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

/* load the next token if it is 'op' or TK_RESERVED. */
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

/* load the next token if it is 'op' */
void expect(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("is not '%c'", op);
  token = token->next;
}

/* load the next token if it is a number. */
int expect_number() {
  if (token->kind != TK_NUM)
    error("is not a number");
  int val = token->val;
  token = token->next;
  return val;
}

/* judge the current token is EOF */
bool at_eof(){
  return token->kind == TK_EOF;
}

/* push the new token into cur destructively.*/
Token *push_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

/* load an array of a charactor into the token */
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  /* seperate the array into the tokens */
  /* load the each token from them.*/
  while (*p) {
    // skip a space
    if (isspace(*p)) {
      p++;
      continue;
    }
    // load the token that is reserved one.
    else if (*p == '+' || *p == '-') {
      cur = push_token(TK_RESERVED, cur, p++);
      continue;
    }
    else if (isdigit(*p)) {
      cur = push_token(TK_NUM, cur, p);
      /* convert the head part of p into number based 10 */
      cur->val = strtol(p, &p, 10);
      continue;
    }
    else {
      error("Can not tokenize.");
    }
  }
  push_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Incorrect number of argument.");
    return 1;
  }
  /* write the argument into a global series of tokens. */
  token = tokenize(argv[1]);

  /**
   * Output the assembly program like this:
   * .intel_syntax noprefix
   * .globl main
   * main:
   *   mov rax, 2
   *   add rax, 3
   *   sub rax, 1
   *   ret
   */

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());
  
  while(!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    } else if (consume('-')) {
      printf("  sub rax, %d\n", expect_number());
      continue;
    } else {
      error("+ or - is expected. %d\n", token->val);
    }
  }
  printf("  ret\n");
  return 0;
}

