#include <ctype.h>
#include <stdarg.h>
/* C99 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

/**
 * Token section.
 */
Token *token;

char *user_input;

/* Output error. This functions is presented in K&R C */
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void error_at(char* loc, char*fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // output an empty charactor pos times
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/* load the next token only if it is 'op' and TK_RESERVED. */
bool consume(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

/**
 * load the next token only if it is 'ident'.
 * move the cursor to the next, and return a current cursor
 */
Token *consume_ident() {
  if(token->kind != TK_IDENT) {
    return NULL;
  }
  Token *cur = token;
  token = token->next;
  return cur;
}

/* load the next token.
 * throw error if it is not 'op.' */
void expect(char *op){
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "is not '%c'", op);
  token = token->next;
}

/* load the next token if it is a number. */
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "is not a number");
  int val = token->val;
  token = token->next;
  return val;
}

/* judge the current token is EOF */
bool at_eof(){
  return token->kind == TK_EOF;
}

/* push the new token into cur destructively.*/
Token *push_token(TokenKind kind, int len, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->len = len;
  tok->str = str;
  cur->next = tok;
  return tok;
}

int tkncmp(const void *s1, const void *s2) {
  return !(memcmp(s1, s2, 2));
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
    } else if (
      // load the token that is reserved one.
      // token with the length is 2
        tkncmp(p, "==") ||
        tkncmp(p, "!=") ||
        tkncmp(p, "<=")
        ) {
      cur = push_token(TK_RESERVED, 2, cur, p);
      p += 2;
      continue;
    } else if (
        *p == '=' ||
        *p == ';' ||
        *p == '+' || 
        *p == '-' || 
        *p == '*' || 
        *p == '/' || 
        *p == '(' || 
        *p == ')' || 
        *p == '<') {
      cur = push_token(TK_RESERVED, 1, cur, p++);
      continue;
    } else if ('a' <= *p && *p <= 'z') {
      // local variable
      // must start with an alphabet
      cur = push_token(TK_IDENT, 1, cur, p);
      char *tmpp = p;
      cur->str = strtok(tmpp, " "); //want to add multi delimiter
      cur->len = strlen(cur->str);
      p += cur->len;
      continue;
    } else if (isdigit(*p)) {
      // HACK: len is not used when a token is a number.
      cur = push_token(TK_NUM, 0, cur, p);
      /* convert the head part of p into number based 10 */
      cur->val = strtol(p, &p, 10);
      continue;
    }
    else {
      // token is not initialized!!
      error_at(p, "Can not tokenize.");
    }
  }
  push_token(TK_EOF, 0, cur, p);
  return head.next;
}

/* stored trees */
Node *code[100];

void program() {
  int i = 0;
  while(!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error_at(token->str, "Incorrect number of argument.");
    return 1;
  }
  user_input = argv[1];
  /* write the argument into a global series of tokens. */
  token = tokenize(argv[1]);

  // consume token and parse it.
  program();

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

  // header
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // get the space of 26 local variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // generate machine code for tree 'node'
  for (int i = 0; code[i]; i++){
    gen(code[i]);
    printf("  pop rax\n");
  }
  // process to return the value
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

