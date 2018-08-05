#define BUF(x) x
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
     TOKEN_COLON_ASSIGN,
     TOKEN_KEYWORD
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

#define buff_hdr(b) (( buffHdr * )( (char *)( ( b ) ) - offsetof(buffHdr,buf) ) )

#define buff_len(b) ( (b)?buff_hdr(b)->len:0 )
#define buff_cap(b) ( (b) ? buff_hdr(b)->cap : 0 )
#define buff_end(b) ( (b) + buff_len(b) )
#define buff_fits(b,n) ( ( buff_len(b) + ( n )) <=  buff_cap(b) )

#define buff_fit(b,n) ( (buff_fits(b,n))?0:((b) = buff_grow((b),buff_len(b)+(n),sizeof(*(b))) ) )


#define buff_push(b,...) ( buff_fit(( b ),1) , (b)[buff_hdr(( b ))->len++] = ( __VA_ARGS__ )  )
#define buff_free(b) ( (( b ))?free(buff_hdr(( b ))):0 )

void *xmalloc(size_t size){
     void *p = malloc(size);
     if ( p ){
          return p;
     } else
          exit(1);
}

void *xcalloc(size_t elem_num, size_t size){
     void *p = calloc(elem_num, size);
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
     for ( int i = 0; i < 20; i++){
          buff_push(xz,i);
     }
     for ( int j = 0; j < 20; j++){
          assert(xz[j] == j );
     }
     return;
}

void syntax_error(const char *fmt, ... ){
     va_list args;
     va_start(args,fmt);
     printf("Syntax Error detected.. \n");
     vprintf(fmt,args);
     va_end(args);
     exit(1);
}
