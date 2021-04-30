#include <stdlib.h>
#include <string.h>
#include "9cc.h"

LVar *locals;
LVar *find_lvar(Token *tok){
  for (LVar *var = locals; var; var = var->next){
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

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


Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

/**
 * 'expr' create a node for an add and sub operation.
 * consumes some tokens for it.
 * return the root node.
 */
Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;
  if (consume("return")) {
    // HACK: only lhs is used to generate code
    node = new_node(ND_RETURN, expr(), NULL);
  } else {
    node = expr();
  }
  expect(";");
  return node;
}


Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("==")) {
        node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
        node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node =  add();
  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LEQ, node, add());
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(ND_LEQ, add(), node);
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
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
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

/**
 * 'primary' represents a node that has not been parsed.
 */
Node *primary() {
  /* matches ( expr ) */
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  } 
  /* matches ident */
  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    // calculate offset of the variable in the stack
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals) {
        lvar->offset = locals->offset + 8;// only 8byte is allowed
      } else {
        lvar->offset = 8;
      }
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }
  /* matches num */
  return new_node_num(expect_number());
}

