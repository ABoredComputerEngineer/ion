
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
     struct{
          BUF( TypeSpec **args;) // Buffer to hold the data type of arguments given to a function ) )
          size_t num_args;
     };
     TypeSpec *ret_type; // The return type of the function
} Func_TypeSpec;

struct TypeSpec{
     TypeSpecKind kind;
     struct {
          const char *name; // Type name
          // array  and pointer
          struct {
               TypeSpec *base_type; // Base type of an array
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
     struct{
        BUF(const char **names); // Buffer for list of names of aggregate types e.g. x,y = int or whatever
        size_t num_names;
     };
     TypeSpec *type;
} AggregateItems;

typedef struct Func_param {
     const char *name;
     TypeSpec *type;
} Func_param;

typedef struct FuncDecl{
     struct {
        BUF(Func_param *params); // Buffer for storing function parameter list
        size_t num_params;
     };
     TypeSpec *ret_type;  // return type of the function
} FuncDecl;

struct Decl{
     DeclKind kind;
     const char *name;
     union {
          struct {
               size_t num_enum_items;
          BUF(EnumItems *enum_items); //Buffer for storing items of enum declarations
          };
          struct {
               size_t num_aggregate_items;
          BUF(AggregateItems *aggregate_items); //Buffer for storing items for structs and unions
          };
          //Typedefs, constant declarations and var declarations
          struct {
               TypeSpec* type;
               Expr* Expr;
          };
          //Function Declarations
          FuncDecl *func_decl;
     };
};


// 
struct Expr {
     ExprKind kind;
     TokenKind op;
     union {
          uint64_t int_val;
          double float_val;
          const char *str_val;
          const char *name;
          //Unary Expressions, Function calls are taken as unary expressions
          struct {
               Expr *operand;
               union{
                    const char *field_name; // field access for members of structs or unions
                    Expr *index;
                    struct {
                         size_t num_args;
                    Expr **args; // buffer to hold function arguments when function callls are used in expressions
                    };
               };

          };
          // Binary Expressions
          struct {
               Expr *left;
               Expr *right;
          };
          //Tenary Expressions
          struct {
               Expr *cond_expr;
               Expr *then_expr;
               Expr *else_expr;
          };

          // Compound Literals
          struct {
               TypeSpec *compound_type;
               struct{
                    size_t num_compound_args;
               BUF(Expr **compound_args); // Buffer
               };
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
     struct {
          size_t num_exprs;
     BUF(Expr **exprs); // buffer to hold a list of expressions
     };
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
               size_t num_cases;
               BUF(Case *cases); // Buffer to store the cases of a switch statement
          };

          // For statement
          struct {
               StmtBlock for_init;
               StmtBlock for_next;
          };
     };

};


// Expressions constructors
Expr *expr_int( uint64_t int_val );
Expr *expr_float( double float_val);
Expr *expr_str( const char *str_val);
Expr *expr_name( const char *name);
Expr *expr_unary( TokenKind op, Expr *expr);
Expr *expr_binary( TokenKind op, Expr *left, Expr *right);
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr);
Expr *expr_case( TypeSpec *type, Expr *expr);
Expr *expr_call(Expr *operand, Expr **args, size_t num_args);
Expr *expr_index(Expr *operand, Expr *index);
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
     new_type->base_type = base;
     new_type->size = size;
     return new_type;
}
TypeSpec *type_pointer(TypeSpec *base){
     TypeSpec *new_type = type_alloc(TYPESPEC_POINTER);
     new_type->base_type = base;
     return new_type;
}

    // BUF( TypeSpec **args;) // Buffer to hold the data type of arguments given to a function ) )
    // TypeSpec *ret_type; // The return type of the function
TypeSpec *type_func(TypeSpec **args,size_t num_args, TypeSpec *ret_type){
     TypeSpec *new_type = type_alloc(TYPESPEC_FUNC);
     new_type->func_decl.args = args;
     new_type->func_decl.num_args = num_args;
     new_type->func_decl.ret_type = ret_type;
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

    Expr *new_expr = expr_alloc( EXPR_BINARY );
    new_expr->op = op;
    new_expr->left=left;
    new_expr->right=right;
    return new_expr; 
}
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr){
     Expr *new_expr = expr_alloc( EXPR_TERNARY );
     
     new_expr->cond_expr = cond;
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

Expr *expr_call(Expr *operand, Expr **args, size_t num_args){
     Expr *new_expr = expr_alloc( EXPR_CALL );
     new_expr->operand = operand;
     new_expr->args = args;
     new_expr->num_args = num_args;
     return new_expr;
}

Expr *expr_index(Expr *operand, Expr *index){
     Expr *new_expr = expr_alloc(EXPR_INDEX);
     new_expr->operand = operand;
     new_expr->index = index;
     return new_expr;
}


Expr *expr_field(Expr *operand, const char *field_name){
     Expr *new_expr = expr_alloc(EXPR_FIELD);
     new_expr->field_name = field_name;
     new_expr->operand = operand;
     return new_expr;
}

Expr *expr_comp( TypeSpec *type, Expr **args ); //TODO

void print_expr(Expr *);

TypeSpec **type_list(size_t count, ... ){
     va_list types;
     va_start(types,count);
     TypeSpec *tmp = NULL;
     TypeSpec **new_type_list = NULL;
     for ( int i = 0; i < count ; i++ ){
          tmp = va_arg(types,TypeSpec *); 
          buff_push(new_type_list,tmp);
     }
     va_end(types);
     return new_type_list;

}
void print_type(TypeSpec *type){
     switch ( type->kind ){
          case TYPESPEC_NAME:
               printf("%s",type->name);
               break;
          case TYPESPEC_ARRAY:
               printf("(array ");
               print_type( type->base_type );
               print_expr(type->size);
               break;
          case TYPESPEC_POINTER:
               printf("(ptr ");
               print_type(type->base_type);
               printf(")");
               break;
          case TYPESPEC_FUNC:{
               Func_TypeSpec fn = type->func_decl;
               printf("(func ");
               for ( TypeSpec **it = fn.args; it != fn.args + fn.num_args ; it++){
                    printf(" ");
                    print_type(*it);
               }
               printf(" ");
               print_type(fn.ret_type); 
               printf(")");
               break;
          }

          
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
          case EXPR_FIELD:
               printf("(field ");
               print_expr(expr->operand);
               printf(" %s)",expr->field_name);
               break;
          case EXPR_INDEX:
               printf("(index ");
               print_expr(expr->operand);
               printf(" ");
               print_expr(expr->index);
               printf(")");
               break;
          case EXPR_CAST:
               printf("(cast ");
               print_type(expr->cast_type);
               printf(" ");
               print_expr(expr->cast_expr);
               printf(")");
               break;
          case EXPR_CALL:
               printf("(");
               print_expr(expr->operand);
               for ( Expr **it = expr->args; it != expr->args + expr->num_args; it++){
                    printf(" ");
                    print_expr(*it);
               }
               printf(")");
               break;
          case EXPR_BINARY:
               printf("(%c ", expr->op);
               print_expr(expr->left);
               printf(" ");
               print_expr(expr->right);
               printf(")");
               break;
          case EXPR_UNARY:
               printf("(%c ", expr->op);
               print_expr(expr->operand);
               printf(")");
               break;
          case EXPR_TERNARY:
               printf("(if ");
               print_expr(expr->cond_expr);
               printf(" ");
               print_expr(expr->then_expr);
               printf(" ");
               print_expr(expr->else_expr);
               printf(")");
               break;
          default:
               assert(0);
               break;

     }
}


Expr **expr_list(size_t count, ... ){
     va_list exprs;
     va_start(exprs,count);
     Expr *tmp = NULL;
     Expr **new_expr_list = NULL;
     for ( int i = 0; i < count ; i++ ){
          tmp = va_arg(exprs,Expr*); 
          buff_push(new_expr_list,tmp);
     }
     va_end(exprs);
     return new_expr_list;

}
void ast_test(){
     Expr **exps = expr_list(3,expr_name("abc"),expr_int(2),expr_int(3)) ;
     TypeSpec **types = type_list(2,type_name("char"),type_name("int"),type_pointer(type_name("int")));


     Expr *expr_list[] = {
          expr_int(1234),
          expr_float(1.234),
          expr_str("FUCK This shit"),
          expr_name("Vector"),
          expr_cast( type_name("int"), expr_int(1234) ),
          expr_binary('+', expr_int(23), expr_name("a") ),
          expr_unary('&',expr_name("variable_name")),
          expr_ternary(expr_binary('=',expr_name("a"),expr_int(2)),expr_name("true"),expr_name("false")),
          expr_call(expr_name("hello"),exps,buff_len(exps)),
          expr_cast(type_func(types,buff_len(types),type_name("int32")),expr_binary('+',expr_name("a"),expr_name("b"))),
          expr_binary('+',expr_field(expr_name("person"),"Age"),expr_int(32)),
          expr_index(expr_name("expr_list"),expr_int(34))
     };
     Expr *new_expr = expr_int( 123 );
     assert( new_expr->kind == EXPR_INT && new_expr->int_val == 123 );

     for ( Expr **it = expr_list; it != expr_list + sizeof(expr_list)/sizeof(Expr *) ; it++ ){
          print_expr( *it );
          putchar('\n');
     }
}
