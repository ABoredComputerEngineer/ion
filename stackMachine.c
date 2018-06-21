#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#define MAX(x,y) ( ( ( x ) < ( y ) )?( y ):( x ) )
#define isOdd(x) (  ( x) & 01 )

typedef enum TokenKind{
     TOKEN_INT = 128,
     TOKEN_NAME
} TokenKind;

typedef struct Token {
     TokenKind kind;
     char *start;
     char *end;
     union {
          int val;
          const char *name;
     };
} Token;

typedef struct buffHdr {
     size_t len;
     size_t cap;
     char buf[0];
} buffHdr;

#define buff_hdr(b) (( buffHdr * )( (char *)( b ) - offsetof(buffHdr,buf) ) )
#define buff_fits(b,n) ( ( buff_len(b) + ( n )) <=  buff_cap(b) )

#define buff_fit(b,n) ( (buff_fits(b,n))?0:((b) = buff_grow((b),buff_len(b)+(n),sizeof(*(b))) ) )


#define buff_len(b) ( (b)?buff_hdr(b)->len:0 )
#define buff_cap(b) ( (b) ? buff_hdr(b)->cap : 0 )
#define buff_push(b,x) ( buff_fit(b,1) , (b)[buff_hdr(b)->len++] = x  )
#define buff_free(b) ( (b)?free(buff_hdr(b)):0 )

void *xmalloc(size_t size){
     void *p = malloc(size);
     if ( p ){
          return p;
     } else
          exit(1);
}

void *xrealloc(void *x, size_t size){
     void *p = realloc(x,size);
     if ( p )
          return p;
     else
          exit(1);
}

void fatal(const char *format, ... ){
     // Error printing function
     va_list args;
     va_start(args,format);
     printf("FATAL : ");
     vprintf(format,args);
     va_end(args);
     exit(1);
}

void *buff_grow(const void *buff, size_t new_len, size_t elem_size ){
     size_t new_cap = MAX( 1 + 2 * buff_cap(buff), new_len);
     size_t new_size = offsetof(buffHdr,buf) + elem_size*new_cap;

     buffHdr *newBuff = NULL; 
     
     if ( buff ){
          newBuff = xrealloc(buff_hdr(buff),new_size);
     } else{
          newBuff = xmalloc(new_size);
          newBuff->len = 0;
     }
     newBuff->cap = new_cap;
     return newBuff->buf;
}

void buff_test(void){
     int *xz = NULL; 
     buff_push(xz,2);
     buff_push(xz, 6);
     buff_push(xz,45);
  //   for ( i = 0; i < 3 ; i++ ){
  //        printf("%d\t",xz[i]);
  //   }
  //   putchar('\n'); 
     buff_free(xz);
     return;
}
const char *str_intern_range( const char *start, const char *end);

// String Interning begins here.....

typedef struct InternStr {
     size_t len;
     const char *str;
} InternStr;

InternStr *intern;

const char *str_intern_range( const char *start, const char *end){
     size_t len = end - start;
     int i;
     for ( i = 0; i < buff_len(intern) ; i++ ){
          if ( (intern[i].len == len) && ( strncmp(intern[i].str,start,len) == 0 ) )
               return intern[i].str;
     }

     char *string = xmalloc(len+1);
     memcpy(string,start,len);
     string[len] = 0;
     InternStr newIntern = {len,string};

     buff_push(intern,newIntern );
     return string;


     
}

const char *str_intern( char *start ){
     return str_intern_range( start, start + strlen(start) ) ;
}

void str_intern_test(void){
     char x[] = "hello";
     char y[] = "hello";
     char z[] = "hellozz";
     assert(x!=y);
     const char *px = str_intern(x);
     const char *py = str_intern(y);
     const char *pz = str_intern(z);
     assert(px == py);
     assert( px != pz );
}

char *stream;
Token token;
void next_token(void){
     int value = 0;
     token.start = stream;
     switch ( *stream ){
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
               token.kind = TOKEN_INT;

               while ( isdigit(*stream) ){
                    value *= 10;
                    value += (*stream - '0');
                    stream++;
               }
               token.val = value;
               break;
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'g':
          case 'h':
          case 'i':
          case 'j':
          case 'k':
          case 'l':
          case 'm':
          case 'n':
          case 'o':
          case 'p':
          case 'q':
          case 'r':
          case 's':
          case 't':
          case 'u':
          case 'v':
          case 'w':
          case 'x':
          case 'y':
          case 'z':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
          case 'G':
          case 'H':
          case 'I':
          case 'J':
          case 'K':
          case 'L':
          case 'M':
          case 'N':
          case 'O':
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
          case 'T':
          case 'U':
          case 'V':
          case 'W':
          case 'X':
          case 'Y':
          case 'Z':
          case '_':
               token.kind = TOKEN_NAME;
               while ( isalpha(*stream) )
                    stream++;
               token.name = str_intern_range(token.start,stream); 
               break;
          case '\\':
               *stream++;
               switch ( *stream ){
                    case 'n':
                         token.kind = '\n';
                         break;
               }
               *stream++;
               break;
          default:
               token.kind = *stream++;
               break;
     }
     token.end = stream;
}

bool is_token(TokenKind kind){  
     // checks if the token is the kind that we want
     return token.kind == kind;
}

bool is_token_name(const char *name){ 
     // checks if the type of token is name and if it is the desired name
     return token.kind == TOKEN_NAME  && token.name == name ; 
}

bool match_token(TokenKind kind){ 
     // Consumes the token and Returns true if the token matches the type. Otherwise just returns false 
     if ( is_token(kind) ){
          next_token();
          return true;
     } else{
          return false;
     }
}

char *token_kind_name(TokenKind kind){
     static char err[100];

     switch ( kind ) {
          case TOKEN_INT:
               sprintf(err,"int");
               break;
          case TOKEN_NAME:
               sprintf(err,"name");
               break;
          default:
               if ( token.kind < 128 && isprint(token.kind) )
                    sprintf(err,"%c",token.kind);  // print the character if it is printable
               else
                    sprintf(err,"%d",token.kind);  // print the ascii value if it is not printable
               break;
     }
     return err;
    
}

bool expect_token(TokenKind kind){
     // Similar to match_token but throws an error if the token if not the kind we expected
     if ( is_token(kind) ){
          next_token();
          return true;
     } else {
          fatal("expected %s got %s\n",token_kind_name(kind), token_kind_name(token.kind) );
          return false;
     }
}


// Parsing Stack Macine bytecode begins here
/* The grammar is as.. .
 *   expr = Action Value '\n'
 *   Action = 'LIT'|'ADD'|'SUB'|'MUL'|'DIV'|'LSHIFT'|'RSHIFR'|'POW'
 *   Value = INT
 *   
 */
typedef enum {
     LIT = 1,
     ADD,
     SUB,
     MUL,
     DIV,
     HALT
} opcode;

typedef struct stack {
     size_t len;
     int val[1024];

} stack;

#define stack_pointer(x) ((stack *)( (char *)base- offsetof(stack,val) ))
#define stack_len(x) (  stack_pointer(x)->len )
#define stack_fits(top,x) ( (stack_len(top) + (x) ) <= 1024)
#define push(x) ( (stack_fits(top,1))?\
          (*(top++) = (x),stack_pointer(top)->len++ ):\
          fatal("Stack Overflow\n") ) 

#define pops(x) ( ( stack_len(base) - (x) ) >= 0 )
#define pop()   ( stack_pointer(base)->len--, *(--top))
void smachine_init(char *str){
     stream = str;
     next_token();
     
}
const char *lit;
const char *add;
const char *sub;
const char *mul;
const char *divide;
const char *halt;
void smachine_str_intern_init(void){
     lit = str_intern("LIT");
     add = str_intern("ADD");
     sub = str_intern("SUB");
     mul = str_intern("MUL");
     divide = str_intern("DIV");
     halt = str_intern("HALT");
}


opcode get_opcode( const char *name ){

     if ( name == lit ){
          return LIT;
     } else if ( name == add ){
          return ADD;
     }else if ( name == sub ){
          return SUB;
     }else if ( name == mul ){
          return MUL;
     }else if ( name == divide ){
          return DIV;
     }else if ( name == halt){
          return HALT;
     }else {
          fatal("Invalid command %s.\n",name);
          return 0;
     }

}

stack machine_stack;
const int *base = &machine_stack.val[0];
int *top = &machine_stack.val[0];


void parse_smachine_expr(void){
     opcode op = get_opcode(token.name);
     next_token(); 
     switch ( op ){
          case LIT: 
               expect_token(' ');
               if ( is_token(TOKEN_INT) ){
                    push(token.val);
                    next_token();
               } else {
                    fatal("Expected 'int' but got '%c' instead ", token.kind );
               }     
               expect_token('\n');
               parse_smachine_expr();      
               break;
          case SUB:
          case ADD:
               if ( !pops(2) ){
                    push(0);
               }
               int rval = pop();
               int lval = pop();
               if ( op == ADD ){
                    push (lval+rval);
               }else{
                    assert( op == SUB );
                    push( lval - rval );
               }
               expect_token('\n');
               parse_smachine_expr();
               break;
          case HALT:
               printf("Program halted by instruction\n");
               break;
          default: 
               fatal("Exprected opcode but got crap instead");
               break;

     }
}

void parse_smachine( ){
     parse_smachine_expr();
}

void smachine_test(void){
     smachine_str_intern_init();
     smachine_init("LIT 1\nLIT 2\nADD\nLIT 1\nSUB\nHALT\n"); 
     parse_smachine();
}


// Parsing simple expressions using recursive descent
//
// Valid operators are +,-,*,/,-(unary),(),
/*
     The grammar for the parser is as follows:
    expr = expr0
    expr0 = expr1 { '+'|'-' expr0 }
    expr1 = expr2 { '*'|'/' expr2 }
    expr2 = expr3 { '^' expr2}
    expr3 = { '-' expr3}|  expr4
    expr4 = INT | '(' expr ')' 
 */
int power_iter( int x, int n , int a ){
     if ( n == 0 ){
          return a;
     } else if ( !isOdd(n) ) {
          return power_iter(x*x,n/2,a );
     } else {
          return power_iter(x,n-1,a*x);
     }
}

int power( int x, int n){
     assert(n>=0);
     return power_iter(x,n,1) ;
}



int parse_expr(void);
int parse_expr0(void);
int parse_expr1(void);
int parse_expr2(void);
int parse_expr3(void);
int parse_expr4(void);

int parse_expr(void){
     return parse_expr0();
}

int parse_expr0(void){
     int val = parse_expr1();
     while ( is_token('+') || is_token('-' ) ){
          char op = token.kind;
          next_token();
          int rval = parse_expr1();
          if ( op == '+' ){
              val += rval; 
          } else {
               assert( op == '-' );
               val -= rval;
          }
     }
     return val;
}

int parse_expr1(void){
     int val = parse_expr2();
     while ( is_token('*') || is_token('/') ){
          char op = token.kind;
          next_token();
          int rval = parse_expr2();
          if ( op == '*' ){
               val *= rval;
          }else {
               assert( op == '/' );
               assert( rval != 0 );
               val /= rval;
          }
     }
     return val;
}

int parse_expr2(void){
     int val = parse_expr3();
     while ( match_token('^') ){
         int rval = parse_expr2();
         val = power(val,rval); 
     }
     return val;
}

int parse_expr3(void){
     if ( match_token('-') ){
          return -parse_expr3();
     } else {
          return parse_expr4();
     }
}

int parse_expr4(void){
     if ( is_token(TOKEN_INT) ){
          int val = token.val;
          next_token();
          return val;
     } else {
          if ( is_token('(') ){
               next_token();
               return parse_expr();
          }
          expect_token(')');
     }
     return token.kind;
}

void init_stream( char *str){
     stream = str;
     next_token();
}

int parse_expr_test(char *str){
     init_stream(str);
     return  parse_expr();
}

#define TOKEN_TEST(x) ( parse_expr_test(#x) == (x) )
void expr_test(void){
       assert(TOKEN_TEST(1+4));
       assert( TOKEN_TEST(1*(2+3) ) );
       assert(TOKEN_TEST(2-3-4-5) );
       assert(TOKEN_TEST(2/3/4) );
       assert( parse_expr_test("--3") == 3 );
       assert( parse_expr_test("2^3") == 8 );
       assert( parse_expr_test("2^2^2") == 16 );
       assert( parse_expr_test("2^7") == 128 );
       assert( parse_expr_test("1+2^2*3") == 13);
       assert( parse_expr_test("2^(1+2)") == 8 );
       assert( parse_expr_test("3^3") == 27 );
}
#undef TOKEN_TEST


void print_token( Token tkn ){
     switch ( tkn.kind ){
          case TOKEN_INT:
               printf("%d\n",tkn.val);
               break;
          case TOKEN_NAME:
               printf("%.*s\n",(int)( tkn.end-tkn.start ), tkn.start );
               break;
          default:
               printf("'%c' \n",tkn.kind );
               break;
     }
}

void lex_test(void){
     char *source = "xy(abc)xy";
     stream = source;
     while ( *stream ){
 //         print_token(token);
          next_token();
     }
//     print_token(token);
}

int main(void){
   //  buff_test();
   //  lex_test();
   //  str_intern_test();
     expr_test();
     smachine_test();
     return 0;
}
