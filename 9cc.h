#include <stdbool.h>
/**
 * Token section.
 */
typedef enum {
  TK_RESERVED,
  TK_RETURN,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token{
  TokenKind kind;
  int len;
  Token *next;
  int val; // value of number
  char *str; 
};

// Local Variable
typedef struct LVar LVar;

struct LVar {
  LVar *next; // LVar is Linked list
  char *name; // the name of this variable (determined by len)
  int len; // the length of this variable
  int offset; // the offset in the stack for this variable to be stored 
};


/* Find the local variable correspond to the token from globally defined 'locals' */
LVar *find_lvar(Token *tok);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *push_token(TokenKind kind, int len, Token *cur, char *str);
Token *tokenize(char *p);

/**
 * Node section.
 * An expression is parsed by this workflow:
 * 
 * program    = stmt*
 * stmt       = expr ";" | "return" expr ";"
 * expr       = assign
 * assign     = equality ("=" assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary)*
 * unary      = ("+" | "-")? primary
 * primary    = num | ident | "(" expr ")"
 */

typedef enum {
  ND_EQ,  // ==
  ND_NEQ, // !=
  ND_LT,  // <
  ND_LEQ, // <=
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_ASSIGN, // =
  ND_RETURN, // return
  ND_LVAR,// local variable
  ND_NUM, // number
} NodeKind;

typedef struct Node Node;

/** 
 * This is a node in a tree of tokens.
 * Node: lhs x rhs -> value
 */
struct Node {
  NodeKind kind; // type of Node
  Node *lhs; // left hand side
  Node *rhs; // right hand side
  int val; // number
  int offset; // an offset from the base pointer to an address of a local variable
};

/**
 * These functions consume tokens to struct a tree.
 */
Node *expr();
Node *assign();
Node *stmt() ;
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/**
 * code generator
 */

void gen_lval(Node *node);
void gen(Node *node);
/**
 * admin interface
 */
void error(char *fmt, ...);
void error_at(char* loc, char*fmt, ...);
