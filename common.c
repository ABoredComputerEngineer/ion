#define BUF(x) x
#define MAX(x,y) ( ( ( x ) < ( y ) )?( y ):( x ) )
#define isOdd(x) (  ( x) & 01 )


#define ARENA_ALIGNMENT 8

#define ALIGN_DOWN(n,a) ( ( ( n ) & ~(( a )-1) ) )
#define ALIGN_UP(n,a) ( ALIGN_DOWN(( n ) + ( a ) - 1,( a )) )
#define ALIGN_DOWN_PTR(p,a) ( (void *)ALIGN_DOWN( (uintptr_t)( p ),( a )) )
#define ALIGN_UP_PTR(p,a) ( (void *)ALIGN_UP((uintptr_t)( p ),( a )) ) 

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
#define buff_free(b) ( (( b ))?free(buff_hdr(( b ))),b = NULL:0 )
#define buff_sizeof(b) ( (b)?buff_len(b)*sizeof(*b):0 )

#define buff_printf(b,...) ( (b) = print_buff( (b) , __VA_ARGS__ ) )
#define buff_clear(b) ( (b)?buff_hdr(b)->len = 0:0 )
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

#define MIN_BUFF_SIZE 16 // the minimum amount of size to request to buffer

void *buff_grow(const void *buff, size_t new_len, size_t elem_size ){
     assert( buff_cap(buff) <= (SIZE_MAX-1)/2 );
     size_t new_cap = MAX( 1 + 2 * buff_cap(buff), MAX(new_len,MIN_BUFF_SIZE));
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
#undef MIN_BUFF_SIZE

char *print_buff(char *buffer, const char *fmt,...){
     va_list args;
     va_start(args,fmt);
     char *dest = buff_end(buffer) ; // dest points to the character NULL  of the string
     size_t cap = buff_cap(buffer) - buff_len(buffer) * sizeof(*buffer);
     size_t new = 1 + vsnprintf(dest,cap,fmt,args);
     size_t len = buff_len(buffer);
     size_t buff_cap = buff_cap(buffer);

     va_end(args);
     if ( new > cap ){
          va_start(args,fmt);
          buff_fit(buffer,new); // increases the size of buffer to fit new string
     len = buff_len(buffer);
     buff_cap = buff_cap(buffer);
          cap = buff_cap(buffer)-buff_len(buffer)*sizeof(*buffer);
          dest = buff_end(buffer) ;
          new = 1+vsnprintf(dest,cap,fmt,args);
          assert(new<=cap);
     }
     buff_hdr(buffer)->len += new-1;
     return buffer;
}



void buff_test(void){
     int *xz = NULL; 
     for ( int i = 0; i < 20; i++){
          buff_push(xz,i);
     }
     for ( int j = 0; j < 20; j++){
          assert(xz[j] == j );
     }

     char *str = NULL;
     buff_printf(str,"%d for all \n",1);
     buff_printf(str,"%s\n","fucc this shit");
     printf("%s",str);
     assert(  str[strlen(str)] == 0 );
     buff_free(str);
     assert(str == NULL);
     buff_free(xz);
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





typedef struct Arena {
     char *ptr; // pointer to the free area of the arena, always aligned to eight bits
     char *end; // pointer to the end of the arena 
     char **blocks; // pointer to the blocks where data is stored.. used to free arena
} Arena;

//#define ARENA_SIZE ( 1024 * 1024 )
#define ARENA_SIZE 1024
void arena_grow(Arena *arena, size_t size ){
     size_t alloc_size = MAX( ARENA_SIZE, ALIGN_UP(size,ARENA_ALIGNMENT) );
     arena->ptr = xmalloc( alloc_size );
     arena->end = arena->ptr + alloc_size ;
     assert( (size_t)(arena->end - arena->ptr) == alloc_size );
     //assert(arena->blocks == NULL );
     buff_push(arena->blocks,arena->ptr);
}


void *arena_alloc(Arena *arena,size_t size ){
     if ( size >= ( size_t )(arena->end-arena->ptr)){
          arena_grow(arena,size);
          assert(size < (size_t)(arena->end-arena->ptr) );
     }

     void *ptr = arena->ptr;
     arena->ptr = ALIGN_UP_PTR( arena->ptr + size , ARENA_ALIGNMENT );
     assert( arena->ptr == ALIGN_DOWN_PTR(arena->ptr,ARENA_ALIGNMENT) );
     assert( arena->ptr <= arena->end );
     assert( ptr == ALIGN_DOWN_PTR(ptr,ARENA_ALIGNMENT) );
     return ptr;          
}

void *arena_dup(Arena *arena,void *src, size_t size ){
     void *dest = arena_alloc(arena,size);
     memcpy(dest,src,size);
     return dest;
}

void arena_free(Arena *arena ){
     for ( char **it = arena->blocks ; it != arena->blocks + buff_len(arena->blocks) ; it++ ){
          free(*it);
     }
     buff_free(arena->blocks);
}


void arena_test(void){
     Arena arena = {};
     arena.blocks = NULL;
     
     int *x = arena_alloc(&arena,sizeof(int)*20);
     for ( int i = 0; i < 20; i++ ){
          x[i] = i;
     }
     int *y = arena_alloc(&arena,sizeof(int) * 30 );
     for ( int i = 0; i < 20 ; i++ ){
          y[i] = 1000 + i;
     }
     for ( int i = 0; i < 20; i++ ){
          printf("%d \t %d \n",x[i],y[i]);
     }
     arena_free(&arena);
}

// String Interning begins here

const char *str_intern(const char *);
const char *str_intern_range( const char *start, const char *end);
Arena arena_intern;
typedef struct Intern {
     size_t len;
     const char *str;
} Intern;

static Intern *intern;

const char *str_intern_range( const char *start, const char *end){
     size_t len = end - start;
     for ( Intern *ip = intern; ip != buff_end(intern) ; ip++ ){
          if ( ip->len == len && ( strncmp(ip->str,start,len) == 0 ) )
               return ip->str;
     }

     char *string = arena_alloc(&arena_intern,len+1);
     memcpy(string,start,len);
     string[len] = 0;
     Intern newIntern = {len,string};

     buff_push(intern,newIntern );
     return string;

     
}

const char *str_intern( const char *start ){
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

/* File Handling */


const char *rev_str_search( const char *stream, const char *key_str ){
     size_t stream_len = strlen(stream);
     size_t key_len = strlen(key_str);
     for ( const char *str = stream+stream_len-1; str != stream; str-- ){
          const char *key = key_str+key_len - 1;
          for ( const char *tmp = str; \
                    key != key_str && *tmp == *key;\
                    tmp--,key--);
          if ( key == key_str ){
               return str - key_len + 1;
          }
     }
     return NULL;

}

void check_error(FILE *fp,const char *func_name){
     if ( ferror(fp) ){
          perror(func_name);
          exit(EXIT_FAILURE);
     }
}

const char *open_ion_file(const char *path){
     FILE *fp = fopen(path, "rb");
     if ( fp == NULL ){
          perror( " fopen() ");
          exit(EXIT_FAILURE);
     }
     const char *extension = rev_str_search(path,".ion");
     if ( extension == NULL ){
          fatal("Invalid file type!\n");
     } 
     if ( fseek( fp, 0, SEEK_END ) != 0 ){
          check_error(fp,"fseek()");
     }
     size_t size = ftell(fp);
     if ( fseek( fp, 0, SEEK_SET) != 0 ){
          check_error(fp,"fseek()");
     }
     char *buffer = xcalloc(size, sizeof(char));
     size_t error = fread(buffer,sizeof(char),size,fp);
     if ( error && error != size ){
          fatal("Corrupted file!\n");
          exit(EXIT_FAILURE);
     }
     fclose(fp);
     return buffer;

}

void *export_to_c(const char *path,const char *out, size_t len){
     const char *extension = rev_str_search(path,".ion");
     size_t path_size = (size_t)(extension - path);
     size_t final_size = path_size + 2 *sizeof(char) + 2;
     char *out_path = xcalloc(final_size,sizeof(char));
     snprintf(out_path,final_size,"%.*s.c",(int)path_size,path);
     FILE *fp  = fopen(out_path,"w");
     if ( fp == NULL ){
          perror( " fopen() ");
          exit(EXIT_FAILURE);
     }
     if ( fwrite(out,sizeof(const char), len,fp )  == 0 ){
          perror("fwrite()");
          fatal("Error writing to file!\n");
     }

     fclose(fp);
}

void file_test(void){
     const char *stream = open_ion_file("./new.ion");
     const char *tmp = "fuck this shit";
     export_to_c("./new.ion",tmp,strlen(tmp)*sizeof(char));
     printf("%s\n",stream);
}
