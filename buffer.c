#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>

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

#define buff_len(b) ( (b)?buff_hdr(b)->len:0 )
#define buff_cap(b) ( (b) ? buff_hdr(b)->cap : 0 )
#define buff_end(b) ( (b) + buff_len(b) )
#define buff_fits(b,n) ( ( buff_len(b) + ( n )) <=  buff_cap(b) )

#define buff_fit(b,n) ( (buff_fits(b,n))?0:((b) = buff_grow((b),buff_len(b)+(n),sizeof(*(b))) ) )


#define buff_push(b,...) ( buff_fit(b,1) , (b)[buff_hdr(b)->len++] = ( __VA_ARGS__ )  )
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
}

void *buff_grow(const void *buff, size_t new_len, size_t elem_size ){
     assert( buff_cap(buff) <= (SIZE_MAX-1)/2 );
     size_t new_cap = MAX( 1 + 2 * buff_cap(buff), new_len);
     assert(new_len<=new_cap);
     assert(new_cap <= ( SIZE_MAX - offsetof(buffHdr,buf) )/elem_size); // new size of the buffer must be smaller than the maximum size of bufer allowed 
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
     assert(buff_len(xz) == 0 );
     return;
}
const char *str_intern_range( const char *start, const char *end);


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

//char *token_kind_name(TokenKind kind){
//     static char err[100];
//
//     switch ( kind ) {
//          case TOKEN_INT:
//               sprintf(err,"int");
//               break;
//          case TOKEN_NAME:
//               sprintf(err,"name");
//               break;
//          default:
//               if ( token.kind < 128 && isprint(token.kind) )
//                    sprintf(err,"%c",token.kind);  // print the character if it is printable
//               else
//                    sprintf(err,"%d",token.kind);  // print the ascii value if it is not printable
//               break;
//     }
//     return err;
//    
//}

size_t copy_token_kind_str( char *dest, size_t dest_size, TokenKind kind ){
     size_t n = 0;
     switch ( kind ){
          case 0:
               n = snprintf(dest,dest_size,"End of File");
               break;
          case TOKEN_INT:
               n = snprintf(dest,dest_size,"integer");
               break;
          case TOKEN_NAME:
               n = snprintf(dest,dest_size,"name");
               break;
          default:
               if ( kind < 128 && isprint(kind) ){
                    n = snprintf(dest,dest_size,"%c",kind);
               }else {
                    n = snprintf(dest,dest_size,"<ASCII %d>",kind);
               }
               break;
     }
     return n;
}

const char *token_kind_str(TokenKind kind){
     static char buf[256];
     size_t n = copy_token_kind_str(buf,sizeof(buf),kind);
     assert( n + 1 <= sizeof(buf) );
     return buf;
}

bool expect_token(TokenKind kind){
     // Similar to match_token but throws an error if the token if not the kind we expected
     if ( is_token(kind) ){
          next_token();
          return true;
     } else {
          char buff[256];
          copy_token_kind_str(buff,sizeof(buff),kind);
          fatal("expected token %s, got %s.\n",buff,token_kind_str(token.kind) );
          return false;
     }
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
     } else  if ( match_token('(') ){
               int val = parse_expr();
               expect_token(')');
               return val;
     } else {
          fatal("expected integer or ( but got %s.\n",token_kind_str(token.kind) );
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

// String Interning begins here.....

typedef struct Intern {
     size_t len;
     const char *str;
} Intern;

static Intern *intern;

const char *str_intern_range( const char *start, const char *end){
     size_t len = end - start;
     int i;
     for ( Intern *ip = intern; ip != buff_end(intern) ; ip++ ){
          if ( ip->len == len && ( strncmp(ip->str,start,len) == 0 ) )
               return ip->str;
     }

     char *string = xmalloc(len+1);
     memcpy(string,start,len);
     string[len] = 0;
     Intern newIntern = {len,string};

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

void run_tests(void){
     buff_test();
     lex_test();
     str_intern_test();
     expr_test();
}

int main(int argc, char **argv){
     run_tests();
     return 0;
}
