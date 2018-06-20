#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>


#define MAX(x,y) ( ( ( x ) < ( y ) )?( y ):( x ) )

typedef enum preced {
     BRACKET = 0,
     SUB = 1,
     ADD,
     MUL,
     DIV,
     POWER
} preced;

typedef enum TokenKind{
     TOKEN_INT = 128,
     TOKEN_NAME,
     TOKEN_OPERATOR,
     TOKEN_BRACKET
} TokenKind;

typedef struct Token {
     TokenKind kind;
     const char *start;
     const char *end;
     union {
          int val;
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


#define buff_len(b) ( (b)?(buff_hdr(b)->len):0 )
#define buff_cap(b) ( (b) ? buff_hdr(b)->cap : 0 )
#define buff_push(b,x) ( buff_fit(b,1) , (b)[buff_hdr(b)->len++] = x  )
#define buff_free(b) ( (b)?free(buff_hdr(b)):0 )

void *buff_grow(const void *buff, size_t new_len, size_t elem_size ){
     size_t new_cap = MAX( 1 + 2 * buff_cap(buff), new_len);
     size_t new_size = offsetof(buffHdr,buf) + elem_size*new_cap;

     buffHdr *newBuff = NULL; 
     
     if ( buff ){
          newBuff = realloc(buff_hdr(buff),new_size);
     } else{
          newBuff = malloc(new_size);
          newBuff->len = 0;
     }
     newBuff->cap = new_cap;
     return newBuff->buf;
}

void buff_test(void){
     int *xz = NULL, i = 0; 
     buff_push(xz,2);
     buff_push(xz, 6);
     buff_push(xz,45);
     for ( i = 0; i < 3 ; i++ ){
          printf("%d\t",xz[i]);
     }
     putchar('\n'); 
     buff_free(xz);
     return;
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
               break;
          case '+':
          case '-':
          case '*':
          case '/':
          case '^':
               token.kind = TOKEN_OPERATOR;
               stream++;
               break;
          case '(':
               token.kind = TOKEN_BRACKET;
               stream++;
               break;
          default:
               token.kind = *stream++;
               break;
     }
     token.end = stream;
}

char *operator;
char **values;
void get_exp(void);
void get_op(void);
void pop(char **, char *);
int precedence(char);


int precedence( char op ){
     switch (op){
          case '+':
               return ADD;
               break;
          case '-':
               return SUB;
               break;
          case '*':
               return MUL;
               break;
          case '/':
               return DIV;
               break;
          case '(':
               return BRACKET;
               break;
          case 0:
               return -1;
               break;
     }
}


void pop(char **val, char *op){
//     size_t debug_len = (3+strlen(val[buff_len(val)-1])+strlen(val[buff_len(val)-2] )+2 ) ;
     
     char *exp = malloc(3+strlen(val[buff_len(val)-1])+strlen(val[buff_len(val)-2] )+2 + 1); 
     //printf("\ndebug_len: %lu\n buff len : \n values : %lu \n operators : %lu\n ",debug_len,buff_len(val),buff_len(op)); 
     sprintf(exp,"(%c %s %s)",op[buff_len(op)-1],val[buff_len(val)-2],val[buff_len(val)-1] );
     free(val[buff_len(val)-1]);
     free(val[buff_len(val)-2]);
     buff_hdr(val)->len-=2;
     buff_push(val,exp);
     buff_hdr(op)->len-=1;
}
void get_exp(){
     next_token();
     int i = 0;
     char *x = malloc( token.end - token.start );
      
     switch ( token.kind ){
          case TOKEN_INT:
               while ( token.start < token.end ){
                    x[i++] = *token.start++;
               }
               x[i] = 0;
               buff_push(values,x);
               get_op();
               break;
          case TOKEN_BRACKET:
               buff_push(operator,*token.start);
               get_exp();
               break;
          case ')':
                    while ( operator[buff_len(operator)-1] != '(' ){
                         pop(values,operator);
                    }
                    buff_hdr(operator)->len--;
               break;
          case 0:
               while ( buff_len(values)!=0 && buff_len(operator)!=1)
                    pop(values, operator);
               break;
     }
               

}

void get_op(){
     next_token();
     switch ( token.kind ){
          case TOKEN_OPERATOR:
               while ( precedence(operator[buff_len(operator)-1]) > precedence(*token.start) )
                    pop(values,operator);
 //                   if ( operator[0] == 0 )
 //                        operator[0] = *token.start;
 //                   else 
                         buff_push(operator,*token.start);
               
               get_exp();
               break;
          case ')':
                    while ( operator[buff_len(operator)-1] != '(' ){
                         pop(values,operator);
                    }
                    buff_hdr(operator)->len--;
                    get_op();
               break;
          case 0:
               while ( buff_len(values)!=0 && buff_len(operator)!=1)
                    pop(values, operator);
              break;
          default:
               break;

     }
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
     char *source = "()+123abc46";
     stream = source;
     while ( *stream ){
          print_token(token);
          next_token();
     }
     print_token(token);
}

int main(void){
     char *exp = "1+(3+3)*7+2*8";
     stream = exp;
     buff_push(operator,0);
     get_exp();
     printf("%s\n",*values);
     free(*values);
     buff_free(values);
     return 0;
}
