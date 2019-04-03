// main.c
/*


*/

#include "main.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// to use vector instead of something
Vector* tokens;
int pos = 0;
Node* code[100];

Node *new_node(NodeType ty, Node *lhs, Node *rhs) {
   Node *node = malloc(sizeof(Node));
   node->ty = ty;
   node->lhs = lhs;
   node->rhs = rhs;
   return node;
}

Node *new_num_node(long num_val) {
   Node *node = malloc(sizeof(Node));
   node->ty = ND_NUM;
   node->num_val = num_val;
   return node;
}

//Map *idents;
static Env *env;
int rsp_offset = 0;

Node *new_ident_node(char* name) {
   Node *node = malloc(sizeof(Node));
   node->ty = ND_IDENT;
   node->name = name;
   rsp_offset += 8;
   map_put(env->idents, name, (void*)8);
   return node;
}

Node *new_func_node(char* name) {
   Node *node = malloc(sizeof(Node));
   node->ty = ND_FUNC;
   node->name = name;
   node->lhs = NULL;
   node->rhs = NULL;
   node->argc = 0;
   return node;
}

Env *new_env(Env* prev_env) {
   Env *env = malloc(sizeof(Env));
   env->env = prev_env;
   env->idents = new_map();
   return env;
}

Node *new_fdef_node(char* name, Env* prev_env) {
   Node *node = malloc(sizeof(Node));
   node->ty = ND_FDEF;
   node->name = name;
   node->lhs = NULL;
   node->rhs = NULL;
   node->argc = 0;
   node->env = prev_env;
   return node;
}

int confirm_node(TokenConst ty) {
   if (tokens->data[pos]->ty != ty) {
      return 0;
   }
   return 1;
}

int consume_node(TokenConst ty) {
   if (tokens->data[pos]->ty != ty) {
      return 0;
   }
   pos++;
   return 1;
}

void tokenize(char *p) {
   tokens = new_vector();
   while (*p != '\0') {
      if (isspace(*p)) {
         p++;
         continue;
      }
      if ((
            *p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '%' || *p == '^' || *p == '|' || *p == '&'
         ) && (*(p+1) == '=')) {
         {
            Token *token = malloc(sizeof(Token));
            token->ty = '=';
            token->input = p+1;
            vec_push(tokens, token);
         }
         {
            Token *token = malloc(sizeof(Token));
            token->ty = TK_OPAS;
            token->input = p;
            vec_push(tokens, token);
         }
         p+=2;
         continue;
      }

      if (( *p == '+' && *(p+1) == '+') || ( *p == '-' && *(p+1) == '-')) {
         Token *token = malloc(sizeof(Token));
         // TK_PLUSPLUS and TK_SUBSUB
         token->ty = *p + *(p+1);
         token->input = p;
         vec_push(tokens, token);
         p+=2;
         continue;
      }

      if (
            *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == ';' || *p == ',' || *p == '{' || *p == '}' ||
            *p == '%' || *p == '^' || *p == '|' || *p == '&' || *p == '?' ||
            *p == ':'
            ) {
         Token *token = malloc(sizeof(Token));
         token->ty = *p;
         token->input = p;
         vec_push(tokens, token);
         p++;
         continue;
      }
      if (*p == '=') {
         Token *token = malloc(sizeof(Token));
         token->input = p;
         if (*(p+1) == '=') {
            token->ty = TK_ISEQ;
            p += 2;
         } else {
            token->ty = '=';
            p++;
         }
         vec_push(tokens, token);
         continue;
      }
      if (*p == '!') {
         Token *token = malloc(sizeof(Token));
         token->input = p;
         if (*(p+1) == '=') {
            token->ty = TK_ISNOTEQ;
            p += 2;
         } else {
            puts("error: not supported");
            exit(1);
         }
         vec_push(tokens, token);
         continue;
      }
      if (*p == '<') {
         Token *token = malloc(sizeof(Token));
         token->input = p;
         if (*(p+1) == '<') {
            token->ty = TK_LSHIFT;
            p += 2;
         } else {
            puts("error: not supported");
            exit(1);
         }
         vec_push(tokens, token);
         continue;
      }
      if (*p == '>') {
         Token *token = malloc(sizeof(Token));
         token->input = p;
         if (*(p+1) == '>') {
            token->ty = TK_RSHIFT;
            p += 2;
         } else {
            puts("error: not supported");
            exit(1);
         }
         vec_push(tokens, token);
         continue;
      }
      if (isdigit(*p)) {
         Token *token = malloc(sizeof(Token));
         token->ty = TK_NUM;
         token->input = p;
         token->num_val = strtol(p, &p, 10);
         vec_push(tokens, token);
         continue;
      }

      if ('a' <= *p && *p <= 'z') {
         Token *token = malloc(sizeof(Token));
         token->ty = TK_IDENT;
         token->input = malloc(sizeof(char)*256);
         vec_push(tokens, token);
         int j=0;
         do {
            token->input[j] = *p;
            p++;
            j++;
         } while(('a' <= *p && *p <= 'z') || ('0' <= *p && *p <= '9'));
         token->input[j] = '\0';

         if (strcmp(token->input, "if") == 0) {
            token->ty = TK_IF;
         }
         if (strcmp(token->input, "else") == 0) {
            token->ty = TK_ELSE;
         }
         continue;
      }

      fprintf(stderr, "Cannot Tokenize: %s\n", p);
      exit(1);
      // DEBUG
      /*
      for (int j=0;j<=tokens->len-1;j++) {
         fprintf(stderr, "%c %d %s\n", tokens->data[j]->ty, tokens->data[j]->ty, tokens->data[j]->input);
      }
      */
   }

   Token *token = malloc(sizeof(Token));
   token->ty = TK_EOF;
   token->input = p;
   vec_push(tokens, token);
}

Node *node_mul();
Node *node_term();
Node *node_mathexpr();
Node *node_or();
Node *node_and();
Node *node_xor();
Node *node_shift();
Node *node_add();
Node *node_cast();

Node *node_mathexpr() {
   return node_or();
}

Node *node_shift() {
   Node *node = node_add();
   while (1) {
      if (consume_node(TK_LSHIFT)) {
         node = new_node(ND_LSHIFT, node, node_add());
      } else if (consume_node(TK_RSHIFT)) {
         node = new_node(ND_RSHIFT, node, node_add());
      } else {
         return node;
      }
   }
}
Node *node_or() {
   Node *node = node_xor();
   while (1) {
      if (consume_node('|')) {
         node = new_node('|', node, node_xor());
      } else {
         return node;
      }
   }
}
Node *node_and() {
   Node *node = node_shift();
   while (1) {
      if (consume_node('&')) {
         node = new_node('&', node, node_shift());
      } else {
         return node;
      }
   }
}
Node *node_xor() {
   Node *node = node_and();
   while (1) {
      if (consume_node('^')) {
         node = new_node('^', node, node_and());
      } else {
         return node;
      }
   }
}
Node *node_add() {
   Node *node = node_mul();
   while (1) {
      if (consume_node('+')) {
         node = new_node('+', node, node_mul());
      } else if (consume_node('-')) {
         node = new_node('-', node, node_mul());
      } else {
         return node;
      }
   }
}

Node *node_cast() {
   if (consume_node(TK_PLUSPLUS)) {
      Node *node = new_ident_node(tokens->data[pos++]->input);
      return new_node(ND_INC, node, NULL);
   } else if (consume_node(TK_SUBSUB)) {
      Node *node = new_ident_node(tokens->data[pos++]->input);
      return new_node(ND_DEC, node, NULL);
   } else {
      return node_term();
   }
}

Node *node_mul() {
   Node *node = node_cast();
   while (1) {
      if (consume_node('*')) {
         node = new_node('*', node, node_cast());
      } else if (consume_node('/')) {
         node = new_node('/', node, node_cast());
      } else if (consume_node('%')) {
         node = new_node('%', node, node_cast());
      } else {
         return node;
      }
   }
}

Node *node_term() {
   if (tokens->data[pos]->ty == TK_NUM) {
      Node *node = new_num_node(tokens->data[pos++]->num_val);
      return node;
   }
   if (tokens->data[pos]->ty == TK_IDENT) {
      Node *node;
      // Function Call
      if (tokens->data[pos+1]->ty == '(') {
         node = new_func_node(tokens->data[pos]->input);
         // skip func , (
         pos += 2;
         while (1) {
            if (!consume_node(',') && consume_node(')')) {
               break;
            }
            node->args[node->argc++] = node_mathexpr();
         }
         assert(node->argc <= 6);
         // pos++ because of consume_node(')')
      } else {
         node = new_ident_node(tokens->data[pos++]->input);
      }
      return node;
   }
   // Parensis
   if (consume_node('(')) {
      Node *node = node_mathexpr();
      if (!consume_node(')')) {
         puts("Error: Incorrect Parensis.");
         exit(1);
      }
      return node;
   }
   printf("Error: Incorrect Parensis without %c %d -> %c %d\n", 
         tokens->data[pos-1]->ty, tokens->data[pos-1]->ty,
         tokens->data[pos]->ty, tokens->data[pos]->ty);
   exit(1);
}

int get_lval_offset(Node *node) {
   int offset = NULL;
   Env *local_env = env;
   while (offset == NULL && local_env != NULL) {
      offset = (int)map_get(local_env->idents, node->name);
      local_env = local_env->env;
   }
   return offset;
}

void gen_lval(Node *node) {
   if (node->ty != ND_IDENT) {
      puts("error: Incorrect Variable of lvalue");
      exit(1);
   }

   //int offset = (int)map_get(env->idents, node->name);
   int offset = get_lval_offset(node);
   puts("mov rax, rbp");
   printf("sub rax, %d\n", offset);
   puts("push rax");
}

void gen(Node *node) {
   if (node->ty == ND_NUM) {
      printf("push %ld\n", node->num_val);
      return;
   }

   if (node->ty == ND_FDEF) {
      printf("%s:\n", node->name);
      for (int j=0; j<node->argc;j++) {
         // read inside functions.
         gen(node->args[j]);
      }
      puts("pop rbp");
      puts("ret");
         // FIXME
         return;

   /*
   puts("push rbp"); 
   puts("mov rbp, rsp");
   printf("sub rsp, %d\n", rsp_offset);
   for (int i=0;code[i];i++) {
      gen(code[i]);
      puts("pop rax");
   }
   puts("mov rsp, rbp");
   */
   }

   if (node->ty == ND_FUNC) {
      char registers[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
      for (int j=0; j<node->argc;j++) {
         gen(node->args[j]);
         printf("pop %s\n", registers[j]);
      }
      // FIXME: alignment should be 64-bit
      printf("call %s\n", node->name);
      puts("push rax");
      return;
   }

   if (node->ty == ND_IDENT) {
      gen_lval(node);
      puts("pop rax");
      puts("mov rax, [rax]");
      puts("push rax");
      return;
   }

   if (node->ty == '=') {
      gen_lval(node->lhs);
      gen(node->rhs);
      puts("pop rdi");
      puts("pop rax");
      puts("mov [rax], rdi");
      puts("push rdi");
      return;
   }

   if (node->ty == ND_INC) {
      gen_lval(node->lhs);
      puts("pop rax");
      puts("mov rdi, [rax]");
      puts("add rdi, 1");
      puts("mov [rax], rdi");
      puts("push rax");
      return;
   }
   if (node->ty == ND_DEC) {
      gen_lval(node->lhs);
      puts("pop rax");
      puts("mov rdi, [rax]");
      puts("sub rdi, 1");
      puts("mov [rax], rdi");
      puts("push rax");
      return;
   }

   gen(node->lhs);
   gen(node->rhs);

   puts("pop rdi");
   puts("pop rax");
   switch (node->ty) {
      case '+':
         puts("add rax, rdi");
         break;
      case '-':
         puts("sub rax, rdi");
         break;
      case '*':
         puts("mul rdi");
         break;
      case '/':
         puts("mov rdx, 0");
         puts("div rax, rdi");
         break;
      case '%':
         puts("mov rdx, 0");
         puts("div rax, rdi");
         // modulo are stored in rdx.
         puts("mov rax, rdx");
         break;
      case '^':
         puts("xor rax, rdi");
         break;
      case '&':
         puts("and rax, rdi");
         break;
      case '|':
         puts("or rax, rdi");
         break;
      case ND_RSHIFT:
         // FIXME: for signed int (Arthmetric)
         // mov rdi[8] -> rax
         puts("mov cl, dil");
         puts("sar rax, cl"); break;
      case ND_LSHIFT:
         puts("mov cl, dil");
         puts("sal rax, cl"); break;
      case ND_ISEQ:
         puts("cmp rdi, rax");
         puts("sete al");
         puts("movzx rax, al");
         break;
      case ND_ISNOTEQ:
         puts("cmp rdi, rax");
         puts("setne al");
         puts("movzx rax, al");
         break;
      default:
         puts("Error");
         exit(1);
   }
   puts("push rax");
}


void expect(int line, int expected, int actual) {
   if (expected == actual)
      return;
   fprintf(stderr, "%d: %d expected, but got %d\n",
         line, expected, actual);
   exit(1);
}

Node *assign() {
   Node *node = node_mathexpr();
   if (consume_node('=')) {
      if (confirm_node(TK_OPAS)) {
         // FIXME: shift
         node = new_node('=', node, new_node(tokens->data[pos++]->input[0], node, assign()));
      } else {
         node = new_node('=', node, assign());
      }
   } else if (consume_node(TK_ISEQ)) {
      node = new_node(ND_ISEQ, node, assign());
   } else if (consume_node(TK_ISNOTEQ)) {
      node = new_node(ND_ISNOTEQ, node, assign());
   }
   return node;
}

Node *stmt() {
   Node *node = assign();
   if (!consume_node(';')) {
      puts("Error: Not token ;");
      exit(1);
   }
   return node;
}
int i = 0;

void program(Node** args) {
   while (!consume_node('}')) {
      if (consume_node('{')) {
         program(args++);
         continue;
      }
      args[i++] = stmt();
   }
}

void toplevel() {
   // funcname: tokens->data[0]->input
   // consume_node('(')
   // : tokens->data[0]->input
   // consume_node(',')
   // ...
   // consume_node('{')
   // idents = new_map...
   // stmt....
   // consume_node('}')
   env = new_env(NULL);
   //idents = new_map();
   while(tokens->data[pos]->ty != TK_EOF) {
      /*
      if (consume_node('}')) {
         continue;
      }
      */
      // FIXME because toplevel func call
      if (tokens->data[pos]->ty == TK_IDENT && tokens->data[pos+1]->ty == TK_IDENT && tokens->data[pos+2]->ty == '(') {
         // expected int func() {
         // skip ')', '{'
         code[i++] = new_fdef_node(tokens->data[pos+1]->input, NULL);
         pos += 5;
         program(code[i++]->args);
         continue;
      }
      if (consume_node('{')) {
         program(&code[i]);
         continue;
      }
      code[i++] = stmt();
   }
   code[i] = NULL;
}

void test_map() {
   Map *map = new_map();
   expect(__LINE__, 0, (int)map_get(map, "foo"));
   map_put(map, "foo", (void*)2);
   expect(__LINE__, 2, (int)map_get(map, "foo"));
}

int main(int argc, char **argv) {
   if (argc < 2) {
      puts("Incorrect Arguments");
      exit(1);
   }

   test_map();

   //Vector *vec = new_vector();
   //vec_push(vec, 9);
   // assert(1 == vec->len);

   tokenize(argv[1]);
   toplevel();
   //Node *node = node_mathexpr();
   // char *p = argv[1];

   puts(".intel_syntax");
   puts(".global main");
   puts("main:");

   puts("push rbp"); 
   puts("mov rbp, rsp");
   printf("sub rsp, %d\n", rsp_offset);
   for (int j=0;code[j];j++) {
      gen(code[j]);
      puts("pop rax");
   }
   puts("mov rsp, rbp");
   puts("pop rbp");
   puts("ret");

   return 0;
}
