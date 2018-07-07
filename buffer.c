#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>

#define MAX(x,y) ( ( ( x ) < ( y ) )?( y ):( x ) )
#define isOdd(x) (  ( x) & 01 )

typedef enum TokenKind{
     TOKEN_EOF = 0,
     TOKEN_LASTCHAR = 127,
     TOKEN_INT = 128,
     TOKEN_FLOAT,
     TOKEN_STR,
     TOKEN_NAME,
     TOKEN_INC,
     TOKEN_DEC,
     TOKEN_LSHIFT,
     TOKEN_RSHIFT,
     TOKEN_EQ,
     TOKEN_NOTEQ,
     TOKEN_LTEQ,
     TOKEN_GTEQ,
     TOKEN_AND,
     TOKEN_OR,
     TOKEN_ADD_ASSIGN,
     TOKEN_MUL_ASSIGN,
     TOKEN_SUB_ASSIGN,
     TOKEN_DIV_ASSIGN,
     TOKEN_MOD_ASSIGN,
     TOKEN_AND_ASSIGN,
     TOKEN_OR_ASSIGN,
     TOKEN_XOR_ASSIGN,
     TOKEN_LSHIFT_ASSIGN,
     TOKEN_RSHIFT_ASSIGN,
     TOKEN_COLON_ASSIGN
} TokenKind;

typedef enum TokenMod{
     TOKENMOD_NONE = 0,
     TOKENMOD_HEX,
     TOKENMOD_DEC,
     TOKENMOD_OCT,
     TOKENMOD_BIN,
     TOKENMOD_CHAR
} TokenMod;

typedef struct Token {
     TokenKind kind;
     TokenMod mod;
     char *start;
     char *end;
     union {
          uint64_t int_val;
          double float_val;
          const char *str_val;
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

void syntax_error(const char *fmt, ... ){
     va_list args;
     va_start(args,fmt);
     printf("Syntax Error detected.. \n");
     vprintf(fmt,args);
     va_end(args);
}
const char *str_intern_range( const char *start, const char *end);


char *stream;
Token token;

uint64_t char_to_digit[256] = {
     ['0'] = 0,
     ['1'] = 1,
     ['2'] = 2,
     ['3'] = 3,
     ['4'] = 4,
     ['5'] = 5,
     ['6'] = 6,
     ['7'] = 7,
     ['8'] = 8,
     ['9'] = 9,
     ['a'] = 10, ['A'] = 10,
     ['b'] = 11, ['B'] = 11,
     ['c'] = 12, ['C'] = 12,
     ['d'] = 13, ['D'] = 13,
     ['e'] = 14, ['E'] = 14,
     ['f'] = 15, ['F'] = 15,
};



void scan_int(void){
     uint64_t val = 0;

     uint64_t digit; 
     uint64_t base = 10;
     token.kind = TOKEN_INT;
     token.mod = TOKENMOD_DEC;
     if ( *stream == '0' ){
          *stream++;
          if ( 'x' == tolower(*stream)  ){
               stream++; 
               token.mod = TOKENMOD_HEX;
               base = 16;
          } else if ( isdigit(*stream) ) {
               token.mod = TOKENMOD_OCT;
               base  = 8;
          } else if ( 'b' == tolower(*stream) ){
               token.mod = TOKENMOD_BIN;
               stream++; 
               base = 2;
          } else {
               if ( *stream == 0 ){ // if number is single zero i.e, "0" 
                    token.int_val = 0;
                    return;
               }
               syntax_error("Invalid integer literal '%c' \n",*stream);
          }
     }

     for ( ; ; ){
         digit = char_to_digit[*stream];
         if ( digit==0 && *stream != '0' ){
              break;
         }
         if ( digit > base ){
              syntax_error("Digit '%c' out of range for base %lu \n",*stream,base);
              digit = 0;
         }
         if ( val > ( UINT64_MAX - digit )/base){ 
              syntax_error("Integer literal overflow\n");
              while ( isdigit(*stream) )
                   stream++;
              val = 0;
         }
         val = val*base+digit;
         stream++;
       
     }
     token.int_val = val;

}

void scan_float(void){
     const char *start = stream;

     while ( isdigit(*stream) ){
          stream++;
     }
     if ( *stream == '.' || tolower(*stream) == 'e' ){
          if ( tolower( *stream ) == 'e' ){
               stream++;
               if ( *stream == '+' || *stream == '-' )
                    *stream++;
          } else 
               stream++;
     } else {
          syntax_error("Expected '.' in a float literal, but got '%c' instead.\n",*stream);
     }

     while ( isdigit(*stream) ){
          *stream++;
     }
     double val = strtod(start,NULL);
     if ( val == HUGE_VAL || val == -HUGE_VAL ){
          syntax_error("Floating point overflow.\n");
     }
     token.kind = TOKEN_FLOAT;
     token.float_val = val;
}

const char escape_to_char[256] = {
     ['n'] = '\n',
     ['r'] = '\r',
     ['t'] = '\t',
     ['v'] = '\v',
     ['b'] = '\b',
     ['a'] = '\a',
     ['\''] = '\'',
     ['\0'] = 0 
};

void scan_char(){
     char val = 0;
     if ( *stream == '\'' ){
          syntax_error("char literal cannot be empty.\n");
     } else if ( *stream == '\\' ){
          stream++;
          val = escape_to_char[*stream];
          if ( val == 0 && *stream != 0 ){
               syntax_error("Invalid character literal escpae '\\%c'.\n",*stream);
          }

     } else {
          val = *stream;
     }
     stream++;
     if ( *stream != '\'' ){
          syntax_error("Expected closing parenthesis for character literal, but got '%c' instead.\n",*stream);
     }
     stream++;
     

     token.kind = TOKEN_INT;
     token.mod = TOKENMOD_CHAR;
     token.int_val = val;
}

void scan_str(void){
     char val = 0;
     char *str = NULL;
     while ( *stream && *stream != '"' ){
          
         if ( *stream == '\\' ){
              stream++;
              val = escape_to_char[*stream];
              if ( val == 0 && *stream != 0 ){
                   syntax_error("Invalid character literal escpae '\\%c'.\n",*stream);
              }

         } else {
              val = *stream;
         }
         stream++;
          buff_push(str,val);     
     }

     if ( *stream != '"' ){
          syntax_error("Unexpected end of file before string closing quotes\n");
     }
     stream++;
     buff_push(str,0);
     token.str_val = str;
     token.kind = TOKEN_STR;
}


#define TOKEN_CASE1(c,c1,k1) \
     case c:\
     token.kind = *stream++;\
     if ( *stream++ == c1 ) {\
          token.kind = k1;\
     }\
     break;

#define TOKEN_CASE2(c,c1,k1,c2,k2) \
     case c:\
     token.kind = *stream++;\
     if ( *stream == c1 ) {\
          stream++;\
          token.kind = k1;\
     } else if ( *stream == c2 ) {\
          stream++;\
          token.kind = k2;\
     }\
     break;

          
void next_token(void){
top:
     token.start = stream;
     switch ( *stream ){
          case ' ': case '\n': case '\r': case '\t': 
               while ( isspace(*stream) )
                    stream++;
               goto top;
               break;
          case '.':
               scan_float();
               break;
          case '\'':
               stream++;
               scan_char();
               break;
          case '"':
               stream++;
               scan_str();
               break;
/*   
 *   CASES FOR MULTI CHARACTER TOKENS LIKE INCREMENT,DECREMENT, LSHIFT, RSHIFT ETC..
#define TOKEN_CASE1(c,c1,k1) \
     case c:\
     token.kind = *stream++;\
     if ( *stream == c1 ) {\
          token.kind == k1;\
     }\
     break;

#define TOKEN_CASE2(c,c1,k1,c2,k2) \
     case c:\
     token.kind = *stream++;\
     if ( *stream == c1 ) {\
          token.kind == k1;\
     } else if ( *stream == c2 ) {\
          token.kind == k2;\
     }\
     break;

 */
          TOKEN_CASE2('+','=',TOKEN_ADD_ASSIGN,'+',TOKEN_INC)
          TOKEN_CASE2('-','=',TOKEN_SUB_ASSIGN,'-',TOKEN_DEC)
          TOKEN_CASE1('*','=',TOKEN_MUL_ASSIGN)
          TOKEN_CASE1('/','=',TOKEN_DIV_ASSIGN)
          TOKEN_CASE1('%','=',TOKEN_MOD_ASSIGN)
          TOKEN_CASE1('^','=',TOKEN_XOR_ASSIGN)
          TOKEN_CASE2('|','=',TOKEN_OR_ASSIGN,'|',TOKEN_OR)
          TOKEN_CASE2('&','=',TOKEN_AND_ASSIGN,'&',TOKEN_AND)
          TOKEN_CASE1('=','=',TOKEN_EQ)
          TOKEN_CASE1('!','=',TOKEN_NOTEQ)
          TOKEN_CASE1(':','=',TOKEN_COLON_ASSIGN);     
          case '<':
               token.kind = *stream++;
               if ( *stream == '<' ){
                   token.kind = TOKEN_LSHIFT;
                   stream++;
                   if ( *stream == '=' ){
                        stream++;
                        token.kind = TOKEN_LSHIFT_ASSIGN;
                   }
               } else if ( *stream == '=' ){
                    stream++;
                    token.kind = TOKEN_LTEQ;
               }
               break;
          case '>':
               token.kind = *stream++;
               if ( *stream == '>' ){
                   token.kind = TOKEN_RSHIFT;
                   stream++;
                   if ( *stream == '=' ){
                        stream++;
                        token.kind = TOKEN_RSHIFT_ASSIGN;
                   }
               } else if ( *stream == '=' ){
                    stream++;
                    token.kind = TOKEN_GTEQ;
               }
               break;
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
               while ( isdigit(*stream) ){
                    stream++;
               }
               if ( *stream == '.' || tolower(*stream) == 'e'){
                    stream = token.start;
                    scan_float();
               } else {
                    stream = token.start;
                    scan_int();
               }
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

#undef TOKEN_CASE1
#undef TOKEN_CASE2

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
          case TOKEN_FLOAT:
               n = snprintf(dest,dest_size,"float");
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
          int val = token.int_val;
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
               printf("%lu\n",tkn.int_val);
               break;
          case TOKEN_NAME:
               printf("%.*s\n",(int)( tkn.end-tkn.start ), tkn.start );
               break;
          default:
               printf("'%c' \n",tkn.kind );
               break;
     }
}

#define assert_token_int(x) ( assert( token.int_val == (x) && token.kind == TOKEN_INT ), next_token())
#define assert_token_float(x) ( assert( token.float_val == (x) && token.kind == TOKEN_FLOAT ), next_token())
#define assert_token_char(x) ( assert( token.int_val == (x)  && match_token(TOKEN_INT) ))
#define assert_token_str(x) ( assert( strcmp(token.str_val,(x) ) == 0 && match_token(TOKEN_STR) ) )
#define assert_token_assign(x) ( assert( token.kind == (x) && match_token(x) ) )
void lex_test(void){
     // Integer parsing test
     init_stream("2345 18446744073709551615 0xf 077 0");
     assert_token_int(2345);
     assert_token_int(18446744073709551615u);
     assert_token_int(0xf);
     assert_token_int(077); 
     assert_token_int(0);


     // Float parsing test
     init_stream("3.14 .32 3e10");
     assert_token_float(3.14);
     assert_token_float(.32);
     assert_token_float(3e10);

     // Parsing character literals
     init_stream("'a' '\\n' ");
     assert_token_char('a');
     assert_token_char('\n');

     //Parsing string literals
     init_stream("\"abcdefghijkl\"");
     assert_token_str("abcdefghijkl");

     //Parsing assignment operators
     init_stream(">>= != ++ >= << *=");
     assert_token_assign(TOKEN_RSHIFT_ASSIGN);
     assert_token_assign(TOKEN_NOTEQ);
     assert_token_assign(TOKEN_INC);
     assert_token_assign(TOKEN_GTEQ);
     assert_token_assign(TOKEN_LSHIFT);
     assert_token_assign(TOKEN_MUL_ASSIGN);


}

#undef assert_token_int
#undef assert_token_float
#undef assert_token_assign
#undef assert_token_char
#undef assert_token_str

void run_tests(void){
     buff_test();
     lex_test();
     str_intern_test();
     //expr_test();
}

int main(int argc, char **argv){
     run_tests();
     return 0;
}
