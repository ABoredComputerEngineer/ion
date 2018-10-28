extern void parse_error( const char *, ... );
typedef enum TokenKind{
     TOKEN_EOF = 0,
     TOKEN_LASTCHAR = 127,
     TOKEN_LPAREN,
     TOKEN_RPAREN,
     TOKEN_LBRACE,
     TOKEN_RBRACE,
     TOKEN_LBRACKET,
     TOKEN_RBRACKET,
     TOKEN_DOT,
     TOKEN_QUESTION,
     TOKEN_COLON,
     TOKEN_COMMA,
     TOKEN_SEMICOLON,
     TOKEN_INT,
     TOKEN_FLOAT,
     TOKEN_STR,
     TOKEN_NAME,
//     TOKEN_INC,
//     TOKEN_DEC,
     // Multiplication opertors
     TOKEN_MUL,
     TOKEN_MOD,
     TOKEN_DIV,
     TOKEN_BAND,
     TOKEN_LSHIFT,
     TOKEN_RSHIFT,
    // Addition operators 
     TOKEN_ADD,
     TOKEN_XOR,
     TOKEN_SUB,
     TOKEN_BOR,
     // Comparison operators
     TOKEN_EQ,
     TOKEN_NOTEQ,
     TOKEN_LTEQ,
     TOKEN_GTEQ,
     TOKEN_AND,
     TOKEN_OR,
     TOKEN_LT,
     TOKEN_GT,
     // Assignment operators
     TOKEN_ASSIGN,
     TOKEN_INC,
     TOKEN_DEC,
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
     TOKEN_COLON_ASSIGN,
     TOKEN_KEYWORD,
     // Unary Operators
     TOKEN_NOT,
     TOKEN_COMPLEMENT
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
     size_t line_number;
     char *line_start;
     union {
          uint64_t int_val;
          double float_val;
          const char *str_val;
          const char *name;
     };
} Token;

bool is_token_keyword(const char *);
const char *typedef_keyword;
const char *const_keyword;
const char *var_keyword;
const char *enum_keyword;
const char *struct_keyword;
const char *union_keyword;
const char *func_keyword;
const char *if_keyword;
const char *else_keyword;
const char *then_keyword;
const char *switch_keyword;
const char *case_keyword;
const char *default_keyword;
const char *while_keyword;
const char *for_keyword;
const char *do_keyword;
const char *continue_keyword;
const char *break_keyword;
const char *return_keyword;
const char *cast_keyword;
const char *sizeof_keyword;
const char *first_keyword;
const char *last_keyword;
//const char *_keyword;
//const char *_keyword;
const char **keywords = NULL;

//#define keyword_intern(name)  name##_keyword  = str_intern(#name); buff_push(keywords,name##_keyword) 

void keyword_intern(char *key ){
     const char *x = str_intern(key);
     buff_push(keywords,x);
}

void init_intern_keyword(void){
     static int init = 0;
     if ( init ){
         return;
     } 
     typedef_keyword  = str_intern("typedef"); buff_push(keywords,typedef_keyword); 
     const_keyword  = str_intern("const"); buff_push(keywords,const_keyword); 
     var_keyword  = str_intern("var"); buff_push(keywords,var_keyword); 
     enum_keyword  = str_intern("enum"); buff_push(keywords,enum_keyword); 
     struct_keyword  = str_intern("struct"); buff_push(keywords,struct_keyword); 
     union_keyword  = str_intern("union"); buff_push(keywords,union_keyword); 
     func_keyword  = str_intern("func"); buff_push(keywords,func_keyword); 
     if_keyword  = str_intern("if"); buff_push(keywords,if_keyword); 
     then_keyword  = str_intern("then"); buff_push(keywords,then_keyword); 
     else_keyword  = str_intern("else"); buff_push(keywords,else_keyword); 
     switch_keyword  = str_intern("switch"); buff_push(keywords,switch_keyword); 
     case_keyword  = str_intern("case"); buff_push(keywords,case_keyword); 
     default_keyword  = str_intern("default"); buff_push(keywords,default_keyword); 
     while_keyword  = str_intern("while"); buff_push(keywords,while_keyword); 
     for_keyword  = str_intern("for"); buff_push(keywords,for_keyword); 
     do_keyword  = str_intern("do"); buff_push(keywords,do_keyword); 
     return_keyword  = str_intern("return"); buff_push(keywords,return_keyword); 
     break_keyword  = str_intern("break"); buff_push(keywords,break_keyword); 
     continue_keyword  = str_intern("continue"); buff_push(keywords,continue_keyword); 
     cast_keyword  = str_intern("cast"); buff_push(keywords,cast_keyword); 
     sizeof_keyword  = str_intern("sizeof"); buff_push(keywords,sizeof_keyword); 
     first_keyword = typedef_keyword;
     last_keyword = sizeof_keyword;
     assert(first_keyword < last_keyword );
  //_keyword  = str_intern(#name); buff_push(keywords,name##_keyword) 
  //_keyword  = str_intern(#name); buff_push(keywords,name##_keyword) 
  //   keyword_intern("default");
  //   keyword_intern("while");
  //   keyword_intern("for");
  //   keyword_intern("do");
  //   keyword_intern("return");
  //   keyword_intern("break");
  //   keyword_intern("continue");
  //   keyword_intern("sizeof");
     init++;
}

bool is_token_keyword(const char *key ){
  //   for ( const char **it = keywords; it != keywords + buff_len(keywords); it++ ){
  //        if ( strcmp(key,*it )  == 0 )
  //             return true;
  //   }
  //   return false;
     return ( key >= first_keyword && key <= last_keyword );
}



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
          stream++;
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
               if ( !isdigit(*stream) ){ // if number is single zero i.e, "0" 
                    token.int_val = 0;
                    return;
               }
               parse_error("Invalid integer literal '%c' \n",*stream);
          }
     }

     for ( ; ; ){
         digit = char_to_digit[(unsigned char)*stream];
         if ( digit==0 && *stream != '0' ){
              break;
         }
         if ( digit > base ){
              parse_error("Digit '%c' out of range for base %"PRIu64"\n",*stream,base);
              digit = 0;
         }
         if ( val > ( UINT64_MAX - digit )/base){ 
              parse_error("Integer literal overflow\n");
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
     if ( *stream == '.' ){
          if ( !isdigit(*(stream+1)) ){
               stream++;
               return;
          }
     } 
     while ( isdigit(*stream) ){
          stream++;
     }
     if ( *stream == '.' || tolower(*stream) == 'e' ){
          if ( tolower( *stream ) == 'e' ){
               stream++;
               if ( *stream == '+' || *stream == '-' )
                    stream++;
          } else 
               stream++;
     } else {
          parse_error("Expected '.' in a float literal, but got '%c' instead.\n",*stream);
     }

     while ( isdigit(*stream) ){
          stream++;
     }
     double val = strtod(start,NULL);
     if ( val == HUGE_VAL || val == -HUGE_VAL ){
          parse_error("Floating point overflow.\n");
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
          parse_error("char literal cannot be empty.\n");
     } else if ( *stream == '\\' ){
          stream++;
          val = escape_to_char[(unsigned char)*stream];
          if ( val == 0 && *stream != 0 ){
               parse_error("Invalid character literal escpae '\\%c'.\n",*stream);
          }

     } else {
          val = *stream;
     }
     stream++;
     if ( *stream != '\'' ){
          parse_error("Expected closing parenthesis for character literal, but got '%c' instead.\n",*stream);
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
              val = escape_to_char[(unsigned char)*stream];
              if ( val == 0 && *stream != 0 ){
                   parse_error("Invalid character literal escpae '\\%c'.\n",*stream);
              }

         } else {
              val = *stream;
         }
         stream++;
          buff_push(str,val);     
     }

     if ( *stream != '"' ){
          parse_error("Unexpected end of file before string closing quotes\n");
     }
     stream++;
     buff_push(str,0);
     token.str_val = str;
     token.kind = TOKEN_STR;
}


#define TOKEN_CASE1(c,k) \
     case c:\
     token.kind = k;\
     stream++;\
     break;\

#define TOKEN_CASE2(c,k,c1,k1) \
     case c:\
     token.kind = k;\
     stream++;\
     if ( *stream == c1 ) {\
          stream++;\
          token.kind = k1;\
     }\
     break;

#define TOKEN_CASE3(c,k,c1,k1,c2,k2)\
     case c:\
     token.kind = k;\
     stream++;\
     if ( *stream == c1 ){\
          token.kind = k1;\
          stream++;\
     } else if ( *stream == c2 ){\
          token.kind = k2;\
          stream++;\
     }\
     break;

          
void next_token(void){
top:
     token.start = stream;
     switch ( *stream ){
          case ' ': case '\n': case '\r': case '\t': 
               while ( isspace(*stream) ){
                    if ( *stream == '\n' ){
                         token.line_number++;
                    }
                    ++stream;
               }
               assert( !isspace(*stream) );
               token.line_start = stream;
               goto top;
               break;
          case '.':
               token.kind = TOKEN_DOT;
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
          case '/':
               stream++;
               token.kind = TOKEN_DIV;
               if ( *stream == '/'){
                    stream++;
                    while ( *stream != '\n' )
                         stream++;
                    goto top;
               } else if ( *stream == '='){
                    token.kind = TOKEN_DIV_ASSIGN;
                    stream++;
               } else if ( *stream == '*' ){
                    while ( true ){
                         stream++;
                         if ( *stream == '*' ){
                              stream++;
                              if ( *stream == '/' ){
                                   goto top;
                              }
                         }
                    }
               }
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
          TOKEN_CASE3('+',TOKEN_ADD,'=',TOKEN_ADD_ASSIGN,'+',TOKEN_INC)
          TOKEN_CASE3('-',TOKEN_SUB,'=',TOKEN_SUB_ASSIGN,'-',TOKEN_DEC)
          TOKEN_CASE2('*',TOKEN_MUL,'=',TOKEN_MUL_ASSIGN)
//          TOKEN_CASE2('/',TOKEN_DIV,'=',TOKEN_DIV_ASSIGN)
          TOKEN_CASE2('%',TOKEN_MOD,'=',TOKEN_MOD_ASSIGN)
          TOKEN_CASE2('^',TOKEN_XOR,'=',TOKEN_XOR_ASSIGN)
          TOKEN_CASE3('|',TOKEN_OR,'=',TOKEN_OR_ASSIGN,'|',TOKEN_OR)
          TOKEN_CASE3('&',TOKEN_BAND,'=',TOKEN_AND_ASSIGN,'&',TOKEN_AND)
          //TOKEN_CASE1('=','=',TOKEN_EQ)
          TOKEN_CASE2('!',TOKEN_NOT,'=',TOKEN_NOTEQ)
          TOKEN_CASE2(':',TOKEN_COLON,'=',TOKEN_COLON_ASSIGN) 
          TOKEN_CASE1('(',TOKEN_LPAREN)    
          TOKEN_CASE1(')',TOKEN_RPAREN)    
          TOKEN_CASE1('{',TOKEN_LBRACE)    
          TOKEN_CASE1('}',TOKEN_RBRACE)    
          TOKEN_CASE1('[',TOKEN_LBRACKET)    
          TOKEN_CASE1(']',TOKEN_RBRACKET)    
          TOKEN_CASE1(';',TOKEN_SEMICOLON)    
          TOKEN_CASE1(',',TOKEN_COMMA)    
          TOKEN_CASE1('?',TOKEN_QUESTION)   
          TOKEN_CASE1('~',TOKEN_COMPLEMENT) 
          //TOKEN_CASE1('.',TOKEN_DOT)    
          //TOKEN_CASE1('{',TOKEN_LPAREN)    
          //TOKEN_CASE1('{',TOKEN_LPAREN)  
          TOKEN_CASE1(0,TOKEN_EOF) 
          case '=':
               token.kind = TOKEN_ASSIGN;
               stream++;
               if ( *stream == '=' ){
                    token.kind = TOKEN_EQ;
                    stream++;
               }
               break;
          case '<':
               token.kind = TOKEN_LT;
               stream++;
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
               token.kind = TOKEN_GT;
               stream++;
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
               while ( isalpha(*stream) || *stream == '_' || isdigit(*stream))
                    stream++;
               token.name = str_intern_range(token.start,stream); 
               token.kind = ( is_token_keyword(token.name) )?TOKEN_KEYWORD:TOKEN_NAME ;
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

bool is_keyword(const char *key){
     return is_token_keyword(key) && (strcmp(token.name,key) == 0);
}
bool is_assign_op(){
     return ( token.kind >= TOKEN_ASSIGN && token.kind <= TOKEN_RSHIFT_ASSIGN );
}

bool is_unary_op(){
     return token.kind == TOKEN_MUL || token.kind == TOKEN_COMPLEMENT || token.kind == TOKEN_BAND || token.kind == TOKEN_SUB|| token.kind == TOKEN_ADD || token.kind == TOKEN_NOT;
}

bool is_mul_op(){
     //return token.kind == '*' || token.kind == '/' || token.kind == TOKEN_LSHIFT || token.kind == TOKEN_RSHIFT || token.kind == '&' || token.kind == '%';
     return token.kind >= TOKEN_MUL && token.kind <= TOKEN_RSHIFT ; 
}
bool is_add_op(){
     return token.kind >= TOKEN_ADD && token.kind <= TOKEN_BOR;
 //    return token.kind == '+' || token.kind == '-' || token.kind == '|' ;
}

bool is_comp_op(){
     return token.kind >= TOKEN_EQ && token.kind <= TOKEN_GT;
//     return token.kind == TOKEN_EQ || token.kind == TOKEN_NOTEQ || token.kind == TOKEN_LTEQ || token.kind == TOKEN_GTEQ || token.kind == '<' || token.kind == '>';
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


bool match_keyword(const char *x){

    //if ( token.kind == TOKEN_KEYWORD && strcmp(token.name,x) == 0 ){
     if ( token.kind == TOKEN_KEYWORD && token.name ==(x) ){  
          next_token();
          return true;
     } else {
          return false;
     }
}

bool expect_keyword(const char *x){
     if ( match_keyword(x) ){
          return true;
     } else {
          parse_error("Expected %s but got %s insted",x,token.name);
          exit(1);
     }
     return false;
}

bool is_decl_keyword(){
     return ( token.kind == TOKEN_KEYWORD && (token.name>=typedef_keyword && token.name <= union_keyword) );
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
//


#define case1(cond,rv)\
      case cond:\
          return ( rv );\
          break; \

char get_token_char( TokenKind kind ){
     switch ( kind ){
          case1(TOKEN_LPAREN,'(') 
          case1(TOKEN_RPAREN,')') 
          case1(TOKEN_LBRACE,'{') 
          case1(TOKEN_RBRACE,'}') 
          case1(TOKEN_LBRACKET,'[') 
          case1(TOKEN_RBRACKET,']') 
          case1(TOKEN_DOT,'.') 
          case1(TOKEN_QUESTION,'?') 
          case1(TOKEN_COLON,':') 
          case1(TOKEN_COMMA,',') 
          case1(TOKEN_SEMICOLON,';') 
          case1(TOKEN_ASSIGN,'=')
          default:
               assert(0);
               break;
     } 
}

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
               n = snprintf(dest,dest_size,"\'%c\'",get_token_char(kind) );
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

void init_stream( char *str){
     stream = str;
     token.line_number = 1;
     next_token();
}
void print_token( Token tkn ){
     switch ( tkn.kind ){
          case TOKEN_INT:
               printf("%"PRIu64"\n",tkn.int_val);
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

