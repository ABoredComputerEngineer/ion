
typedef struct Decl Decl;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef enum TypeSpecKind TypeSpecKind;
typedef struct TypeSpec TypeSpec;

enum TypeSpecKind {
     TYPESPEC_NAME,
     TYPESPEC_PAREN,
     TYPESPEC_FUNC,
     TYPESPEC_ARRAY,
     TYPESPEC_POINTER
};

typedef struct Func_TypeSpec{
     BUF( TypeSpec **args;) // Buffer to hold the data type of arguments given to a function ) )
     TypeSpec *ret_type; // The return type of the function
} Func_TypeSpec;

struct TypeSpec{
     TypeSpecKind kind;
     union {
          const char *name; // Type name
          // array  and pointer
          struct {
               TypeSpec *base; // Base type of an array
               Expr *size; // Index of the array
          };
          Func_TypeSpec func_decl;
     };
};


typedef enum DeclKind{
     DECL_NONE,
     DECL_ENUM,
     DECL_STRUCT,
     DECL_UNION,
     DECL_VAR,
     DECL_CONST,
     DECL_TYPEDEF,
     DECL_FUNC
} DeclKind;

typedef enum ExprKind {
     EXPR_NONE,
     EXPR_INT,
     EXPR_FLOAT,
     EXPR_STR,
     EXPR_NAME,
     EXPR_CAST,
     EXPR_CALL, // Function calls
     EXPR_INDEX,
     EXPR_FIELD,
     EXPR_UNARY,
     EXPR_BINARY,
     EXPR_TERNARY
} ExprKind;

typedef enum StmtKind {
     STMT_NONE,
     STMT_AUTO_ASSIGN,
     STMT_ASSIGN,
     STMT_FOR,
     STMT_WHILE,
     STMT_DO_WHILE,
     STMT_IF,
     STMT_SWITCH,
     STMT_RETURN,
     STMT_BREAK,
     STMT_CONTINUE
} StmtKind;

typedef struct EnumItems {
     const char *name;
     TypeSpec *type;
} EnumItems;

typedef struct AggregateItems{
     BUF(const char **names); // Buffer for list of names of aggregate types e.g. x,y = int or whatever
     TypeSpec *type;
} AggregateItems;

typedef struct Func_param {
     const char *name;
     TypeSpec *type;
} Func_param;

typedef struct FuncDecl{
     BUF(Func_param *params); // Buffer for storing function parameter list
     TypeSpec *ret_type;  // return type of the function
} FuncDecl;

struct Decl{
     DeclKind kind;
     const char *name;
     union {
          BUF(EnumItems *enum_items); //Buffer for storing items of enum declarations
          BUF(AggregateItems *aggregate_items); //Buffer for storing items for structs and unions
          //Typedefs, constant declarations and var declarations
          struct {
               TypeSpec* type;
               Expr* Expr;
          };
          //Function Declarations
          FuncDecl *func_decl;
     };
};

struct Expr {
     ExprKind kind;
     TokenKind op;
     union {
          uint64_t int_val;
          double float_val;
          const char *str_val;
          const char *name;
          //Unary Expressions
          struct {
               Expr *operand;
               union{
                    const char *field; // field access for members of structs or unions
                    Expr *index;
                    const char **args; // buffer to hold function arguments when function callls are used in expressions
               };

          };
          // Binary Expressions
          struct {
               Expr *left;
               Expr *right;
          };
          //Tenary Expressions
          struct {
               Expr *cond;
               Expr *then_expr;
               Expr *else_expr;
          };

          // Compound Literals
          struct {
               TypeSpec *compound_type;
               BUF(Expr **compound_args); // Buffer
          };

          //Cast
          struct {
               TypeSpec *cast_type;
               Expr *cast_expr; 
          };

     };
};

typedef struct StmtBlock {
     BUF(Stmt **stmts); // Buffer to hold a list of statements
} StmtBlock;

typedef struct Elseif {
     Expr *cond;
     StmtBlock elseif_block;
} Elseif;

typedef struct Case {
     BUF(Expr **exprs); // buffer to hold a list of expressions
     StmtBlock case_block;
} Case;


struct Stmt{
     StmtKind kind;
     Expr *expr; //  for statements like for, while ,return, switch do while etc, cond expression for if .
     StmtBlock block; // Statements block for for, while... statements, if block for if statement.

     union {

          // if statement
          struct {
               Elseif *elseifs; //buffer to store any number of else if blocks.
               StmtBlock else_block;
          };

          // Auto assign
          struct {
               const char *name;
          };
          struct {
               Expr *rhs;
          };

          //Switch statement
          struct {
               BUF(Case *cases); // Buffer to store the cases of a switch statement
          };

          // For statement
          struct {
               StmtBlock for_init;
               StmtBlock for_next;
          };
     };

};



Expr *expr_int( uint64_t int_val );
Expr *expr_float( double float_val);
Expr *expr_str( const char *str_val);
Expr *expr_name( const char *name);
Expr *expr_unary( TokenKind op, Expr *expr);
Expr *expr_binary( TokenKind op, Expr *left, Expr *right);
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr);
Expr *expr_case( TypeSpec *type, Expr *expr);
Expr *expr_call(Expr *operand, const char **args);
Expr *expr_comp( TypeSpec *type, Expr **args );


// TypeSpec constructors

TypeSpec *type_alloc(TypeSpecKind kind){
     TypeSpec *new = xcalloc(1,sizeof(TypeSpec));
     new->kind = kind;
     return new;
}

TypeSpec *type_name(const char *name){
     TypeSpec *new_type =  type_alloc(TYPESPEC_NAME);
     new_type->name = name;
     return new_type;
}

TypeSpec *type_array(TypeSpec *base, Expr *size){
     TypeSpec *new_type = type_alloc(TYPESPEC_ARRAY);
     new_type->base = base;
     new_type->size = size;
     return new_type;
}
TypeSpec *type_pointer(TypeSpec *base){
     TypeSpec *new_type = type_alloc(TYPESPEC_POINTER);
     new_type->base = base;
     return new_type;
}
TypeSpec *type_func(Func_TypeSpec func_decl){
     TypeSpec *new_type = type_alloc(TYPESPEC_FUNC);
     new_type->func_decl = func_decl;
     return new_type;
}


Expr *expr_alloc( ExprKind kind ){
     Expr *new_expr = xcalloc(1,sizeof(Expr));
     new_expr->kind = kind;
     return new_expr;
}
Expr *expr_int( uint64_t int_val ){
     Expr *new_expr = expr_alloc(EXPR_INT);
     new_expr->int_val = int_val;
     return new_expr;
}
Expr *expr_float( double float_val){
     Expr *new_expr = expr_alloc(EXPR_FLOAT);
     new_expr->float_val = float_val;
     return new_expr;
}


Expr *expr_str( const char *str_val){
     Expr *new_expr = expr_alloc( EXPR_STR );
     new_expr->str_val = str_val;
     return new_expr;
}


Expr *expr_name( const char *name){
     Expr *new_expr = expr_alloc( EXPR_NAME );
     new_expr->name = name;
     return new_expr;
}

Expr *expr_unary( TokenKind op, Expr *expr){
    Expr *new_expr = expr_alloc( EXPR_UNARY );
    new_expr->op = op;
    new_expr->operand = expr;
    return new_expr;; 
}

Expr *expr_binary( TokenKind op, Expr *left, Expr *right){

    Expr *new_expr = expr_alloc( EXPR_UNARY );
    new_expr->op = op;
    new_expr->left=left;
    new_expr->right=right;
    return new_expr; 
}
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr){
     Expr *new_expr = expr_alloc( EXPR_TERNARY );
     
     new_expr->cond = cond;
     new_expr->then_expr= then_expr;
     new_expr->else_expr= else_expr;
     return new_expr;
}
Expr *expr_cast( TypeSpec *type, Expr *expr){
     Expr *new_expr = expr_alloc( EXPR_CAST );
     new_expr->cast_type = type;
     new_expr->cast_expr = expr;
     return new_expr;
}
Expr *expr_comp( TypeSpec *type, Expr **args ); //TODO
void print_expr(Expr *);
void print_type(TypeSpec *type){
     switch ( type->kind ){
          case TYPESPEC_NAME:
               printf("%s",type->name);
               break;
          case TYPESPEC_ARRAY:
               printf("(array ");
               print_type( type->base );
               print_expr(type->size);
               break;
          case TYPESPEC_FUNC:
               break;

          
     }

}

void print_expr( Expr *expr ) {
     
     switch ( expr->kind ) {
          case EXPR_INT:
               printf("%"PRIu64"",expr->int_val);
               break;
          case EXPR_FLOAT:
               printf("%f",expr->float_val);
               break;
          case EXPR_STR:
               printf("\"%s\"",expr->str_val);
               break;
          case EXPR_NAME:
               printf("%s",expr->name);
               break;
          case EXPR_CAST:
               printf("(cast ");
               print_type(expr->cast_type);
               printf(" ");
               print_expr(expr->cast_expr);
               printf(")");
               break;

     }
}


void ast_test(){

     Expr *expr_list[] = {
          expr_int(1234),
          expr_float(1.234),
          expr_str("FUCK This shit"),
          expr_name("Vector"),
          expr_cast( type_name("int"), expr_int(1234) )
     };
     Expr *new_expr = expr_int( 123 );
     assert( new_expr->kind == EXPR_INT && new_expr->int_val == 123 );

     for ( Expr **it = expr_list; it != expr_list + sizeof(expr_list)/sizeof(Expr *) ; it++ ){
          print_expr( *it );
          putchar('\n');
     }
}
