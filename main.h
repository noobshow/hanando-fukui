#ifndef __HANANDO_FUKUI_MAIN__
#define __HANANDO_FUKUI_MAIN__

typedef enum {
   TK_ADD = '+',
   TK_SUB = '-',
   TK_EQUAL = '=',
   TK_BLOCKBEGIN = '{',
   TK_BLOCKEND = '}',
   TK_NUM = 256,
   TK_IDENT,
   TK_EOF,
   TK_ISEQ,
   TK_ISNOTEQ,
   TK_IF,
   TK_ELSE,
} TokenConst;

typedef enum {
   ND_ADD = '+',
   ND_SUB = '-',
   ND_MUL = '*',
   ND_DIV = '/',
   ND_LEFTPARENSIS = '(',
   ND_RIGHTPARENSIS = ')',
   ND_NUM = 256,
   ND_IDENT,
   ND_ISEQ,
   ND_ISNOTEQ,
   ND_FUNC,
   ND_FDEF,
} NodeType;

typedef struct Node {
   NodeType ty;
   struct Node *lhs;
   struct Node *rhs;
   struct Node *args[6];
   int argc;
   long num_val;
   char* name;
} Node;

typedef struct {
   TokenConst ty;
   long num_val;
   char *input;
} Token;

typedef struct {
   Token** data;
   int capacity;
   int len;
} Vector;

typedef struct {
   Vector* keys;
   Vector *vals;
} Map;
Vector *new_vector();
void vec_push(Vector *vec, Token* element);

#endif /* __HANANDO_FUKUI_MAIN__ */
