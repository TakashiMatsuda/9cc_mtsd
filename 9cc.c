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
    else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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
      // token is not initialized!!
      error_at(p, "Can not tokenize.");
    }
  }
  push_token(TK_EOF, cur, p);
  return head.next;
}

/**
 * Node part.
 * An expression is parsed by this workflow:
 *
 * primary := ( expr ) | expr | num
 * num := [1, 2, ...]
 * expr := add | mul
 * add := primary + primary
 * mul := primary * primary
 */

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // number
} NodeKind;

typedef struct Node Node;

/** 
 * This is a node in a tree of tokens.
 */
struct Node {
  NodeKind kind; // type of Node
  Node *lhs; // left hand side
  Node *rhs; // right hand side
  int val; // number
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/**
 * These functions consume tokens to struct a tree.
 */
Node *primary();
Node *mul();
Node *expr();

/**
 * 'primary' represents a node that has not been parsed.
 */
Node *primary() {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }
  return new_node_num(expect_number());
}

/**
 * 'expr' create a node for an add and sub operation.
 * consumes some tokens for it.
 * return the root node.
 */
Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume ('-')) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

/**
 * 'mul' create a node for the root of multiplying operation.
 * consumes some tokens for it.
 * return the root node.
 */
Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*')) {
      node = new_node(ND_MUL, node, primary());
    } else if (consume('/')) {
      node = new_node(ND_DIV, node, primary());
    } else {
      return node;
    }
  }
}


/**
 * generate 'push' and 'pop' code for the given node
 */
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  // generate 'push' for a left hand side and right one
  gen(node->lhs);
  gen(node->rhs);

  // generate 'pop'
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  default:
    error("unexpected error: unexpected node kind");
  }

  printf("  push rax\n");
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
  Node *node = expr();

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

  // generate machine code for tree 'node'
  gen(node);

  // process to return the value
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

