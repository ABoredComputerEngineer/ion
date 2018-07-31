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
     EXPR_TERNARY,
     EXPR_COMPOUND

} ExprKind;

typedef enum StmtKind {
     STMT_NONE,
     STMT_AUTO_ASSIGN,
     STMT_OP_ASSIGN,
     STMT_FOR,
     STMT_WHILE,
     STMT_DO_WHILE,
     STMT_IF,
     STMT_SWITCH,
     STMT_RETURN,
     STMT_BREAK,
     STMT_CONTINUE
} StmtKind;

typedef struct enum_item{
     const char *name;
     Expr *expr;
     TypeSpec *type;
} enum_item;

typedef struct aggregate_item{
     BUF(const char **name_list); // Buffer for list of names of aggregate types e.g. x,y = int or whatever
     size_t num_names;
     TypeSpec *type;
} aggregate_item;

typedef struct func_param {
     const char *name;
     TypeSpec *type;
} func_param;

typedef struct func_def{
     BUF(func_param **param_list); // Buffer for storing function parameter list
     size_t num_params;
     TypeSpec *ret_type;  // return type of the function
} func_def;

typedef struct enum_def{
     size_t num_enum_items;
     enum_item **enum_items; // Buffer for storing items of enum declaration,e.g. ( name : expr )
} enum_def;

typedef struct aggregate_def{
     size_t num_aggregate_items;
     aggregate_item **aggregate_items; // Buffer to store the name value pairs for structures and unions``
} aggregate_def;

typedef struct typedef_def{
     TypeSpec *type;
//     Expr *expr; // MOD: Removed expr type for typedef declarations
} typedef_def;

typedef struct const_def{
     Expr *expr;
} const_def;

typedef struct var_def {
     TypeSpec *type;
     Expr *expr;
} var_def;

struct Decl{
     DeclKind kind;
     const char *name;
     union {
          enum_def enum_decl;
          aggregate_def aggregate_decl;
          typedef_def typedef_decl;
          const_def const_decl;
          var_def var_decl;
          func_def func_decl;
     };
};


// 
//
typedef struct unary_def {
     Expr *operand;
} unary_def;

typedef struct field_def{
     Expr *operand;
     const char *field_name;
} field_def;

typedef struct array_def{
     Expr *operand;
     Expr *index;
} array_def;

typedef struct func_call_def{
     Expr *operand;
     size_t num_args;
     Expr **args;
} func_call_def;

typedef struct binary_def{
     Expr *left;
     Expr *right;
} binary_def;

typedef struct ternary_def {
     Expr *cond_expr;
     Expr *then_expr;
     Expr *else_expr;
} ternary_def;

typedef struct compound_def{
     TypeSpec *type;
     size_t num_args;
     BUF(Expr **args); // Buffer
} compound_def;

typedef struct cast_def{
     TypeSpec *cast_type;
     Expr *expr;
} cast_def;
struct Expr {
     ExprKind kind;
     TokenKind op;
     union {
          uint64_t int_val;
          double float_val;
          const char *str_val;
          const char *name;
          unary_def unary_expr;
          array_def array_expr;
          field_def field_expr;

          binary_def binary_expr;
          ternary_def ternary_expr;
          
          cast_def cast_expr;
          compound_def compound_expr;

          func_call_def func_call_expr;
     };
};

typedef struct StmtBlock {
     size_t num_stmts;
     BUF(Stmt **stmts); // Buffer to hold a list of statements
} StmtBlock;


typedef struct Elseif {
     Expr *cond;
     StmtBlock elseif_block;
} Elseif;

typedef struct stmt_if_def {
     Expr *cond;  // the condition for if statement
     size_t num_elseifs;
     Elseif *elseifs; // list of pointers to else_if blocks
     StmtBlock else_block;
} stmt_if_def;

typedef struct stmt_auto_assign_def { // Colon assign
     const char *name;
     Expr *rhs;
} stmt_auto_assign_def;

typedef struct stmt_op_assign_def {
     Expr *lhs;
     TokenKind op;
//     const char *name;
     Expr *rhs;
}  stmt_op_assign_def;

typedef struct Case {
     Expr *case_expr; // buffer to hold a list of expressions
     StmtBlock case_block;
} Case;

typedef struct stmt_switch_def{
     Expr *expr;
     StmtBlock default_block;
     size_t num_cases;
    BUF( Case *cases;) // buffer to hold all the case statements;
} stmt_switch_def;

typedef struct stmt_for_def{
     Expr *expr_init;
     Expr *expr_update;
     Expr *expr_cond;
} stmt_for_def;

typedef struct stmt_return_def {
     Expr *return_expr;
} stmt_return_def;

typedef struct stmt_while_def {
     Expr *expr_cond;
} stmt_while_def;

struct Stmt{
     StmtKind kind;
//     Expr *expr; //  for statements like for, while ,return, switch do while etc, cond expression for if .
     StmtBlock block; // Statements block for for, while... statements, if block for if statement.

     union {
          stmt_return_def return_stmt;
          stmt_while_def while_stmt;
                    
          stmt_if_def if_stmt;
          stmt_for_def for_stmt;
          stmt_switch_def switch_stmt;
          stmt_auto_assign_def auto_assign_stmt;
          stmt_op_assign_def op_assign_stmt;
     };

};


// Expressions constructors [exprcon]
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
Expr *expr_compound( TypeSpec *type, Expr **args, size_t num_args );


// TypeSpec constructors [typecon]

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


Expr *expr_new( ExprKind kind ){
     Expr *new_expr = xcalloc(1,sizeof(Expr));
     new_expr->kind = kind;
     return new_expr;
}
Expr *expr_int( uint64_t int_val ){
     Expr *new_expr = expr_new(EXPR_INT);
     new_expr->int_val = int_val;
     return new_expr;
}
Expr *expr_float( double float_val){
     Expr *new_expr = expr_new(EXPR_FLOAT);
     new_expr->float_val = float_val;
     return new_expr;
}


Expr *expr_str( const char *str_val){
     Expr *new_expr = expr_new( EXPR_STR );
     new_expr->str_val = str_val;
     return new_expr;
}


Expr *expr_name( const char *name){
     Expr *new_expr = expr_new( EXPR_NAME );
     new_expr->name = name;
     return new_expr;
}

Expr *expr_unary( TokenKind op, Expr *expr){
    Expr *new_expr = expr_new( EXPR_UNARY );
    new_expr->op = op;
    new_expr->unary_expr.operand = expr;
    return new_expr;; 
}

Expr *expr_binary( TokenKind op, Expr *left, Expr *right){

    Expr *new_expr = expr_new( EXPR_BINARY );
    new_expr->op = op;
    new_expr->binary_expr.left=left;
    new_expr->binary_expr.right=right;
    return new_expr; 
}
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr){
     Expr *new_expr = expr_new( EXPR_TERNARY );
     
     new_expr->ternary_expr.cond_expr = cond;
     new_expr->ternary_expr.then_expr= then_expr;
     new_expr->ternary_expr.else_expr= else_expr;
     return new_expr;
}
Expr *expr_cast( TypeSpec *type, Expr *expr){
     Expr *new_expr = expr_new( EXPR_CAST );
     new_expr->cast_expr.cast_type = type;
     new_expr->cast_expr.expr = expr;
     return new_expr;
}

Expr *expr_call(Expr *operand, Expr **args, size_t num_args){
     Expr *new_expr = expr_new( EXPR_CALL );
     new_expr->func_call_expr.operand = operand;
     new_expr->func_call_expr.args = args;
     new_expr->func_call_expr.num_args = num_args;
     return new_expr;
}

Expr *expr_index(Expr *operand, Expr *index){
     Expr *new_expr = expr_new(EXPR_INDEX);
     new_expr->array_expr.operand = operand;
     new_expr->array_expr.index = index;
     return new_expr;
}


Expr *expr_field(Expr *operand, const char *field_name){
     Expr *new_expr = expr_new(EXPR_FIELD);
     new_expr->field_expr.field_name = field_name;
     new_expr->field_expr.operand = operand;
     return new_expr;
}

Expr *expr_compound( TypeSpec *type, Expr **args, size_t num_args ){
     Expr *new_expr = expr_new(EXPR_COMPOUND);
     new_expr->compound_expr.type = type;
     new_expr->compound_expr.args = args;
     new_expr->compound_expr.num_args = num_args;
     return new_expr;
}


// Print Functios [prnfunc]
void print_expr(Expr *);
void print_type(TypeSpec *);
void print_decl(Decl *);
void print_stmt(Stmt *);
void print_name_list( const char **names, size_t num_names ){
     for ( const char **it = names; it!=names+num_names; it++ ){
          printf("%s%c ",*it, (it==names+num_names - 1)?' ':',' );
     }
}

int indent = 0;
char *indent_string = "\t\t\t\t\t\t\t\t\t\t";
typedef enum operators {
     INC = 200,
     DEC,
     LSHIFT,
     RSHIFT,
     EQ,
     NOTEQ,
     LTEQ,
     GTEQ,
     AND,
     OR,
     ADD_ASSIGN,
     MUL_ASSIGN,
     SUB_ASSIGN,
     DIV_ASSIGN,
     MOD_ASSIGN,
     AND_ASSIGN,
     OR_ASSIGN,
     XOR_ASSIGN,
     LSHIFT_ASSIGN,
     RSHIFT_ASSIGN,
     COLON_ASSIGN,
     ASSIGN,
} operators;

char *operations[] = {
     [ LSHIFT] = "<<",
     [ RSHIFT] = ">>",
     [ EQ ] = "==",
     [ NOTEQ ] = "!=",
     [ LTEQ ] = "<=",
     [ GTEQ ] = ">=",
     [ AND ] = "&&",
     [ OR ] = "||",
     [ ADD_ASSIGN ] = "+=",
     [ MUL_ASSIGN ] = "*=",
     [ SUB_ASSIGN ] = "-+",
     [ DIV_ASSIGN ] = "/=",
     [ MOD_ASSIGN ] = "%=",
     [ AND_ASSIGN ] = "&=",
     [ OR_ASSIGN ] = "|=",
     [ XOR_ASSIGN ] = "^=",
     [ LSHIFT_ASSIGN ] = "<<=",
     [ RSHIFT_ASSIGN ] = ">>=",
     [ COLON_ASSIGN ] = ":=",
     [ '=' ] = "=",
     ['+'] = "+",
     ['-'] = "-",
     [ '*' ] = "*",
     [ '/' ] = "/",
     [ '%' ] = "%" 
};

void print_stmt_block(StmtBlock block,int indent){
     for ( Stmt **stmt = block.stmts; stmt != block.stmts + block.num_stmts ; stmt++ ){
          putchar('\n');
          printf("%.*s",indent+1,indent_string);
          print_stmt(*stmt);
     }
}
void print_stmt(Stmt *stmt){
     switch ( stmt->kind ) {
          case STMT_IF:
               {
                    stmt_if_def tmp = stmt->if_stmt;
                    printf("(if ");
                    print_expr(tmp.cond);
                    printf("\n(then ");
                    print_stmt_block(stmt->block,indent++);
                    printf(")\n");
                    indent--;
                    if ( tmp.elseifs != NULL ){
                         for ( Elseif *it = tmp.elseifs; it != tmp.elseifs + tmp.num_elseifs ; it++){
                              printf("( elseif ");
                              print_expr( (it)->cond);
                              printf("\n\t(then ");
                              print_stmt_block( (it)->elseif_block, indent++ );
                              indent--;
                              printf(")\n");
                         }
                    }
                    if ( tmp.else_block.num_stmts != 0 ){
                         printf("(else ");
                         print_stmt_block(tmp.else_block,indent++);
                         printf(")");
                    }
                    indent-=1;
                    printf(")");
                    break;
               }
          case STMT_BREAK:
               printf("( break )");
               break;
          case STMT_CONTINUE:
               printf("( continue )");
               break;
          case STMT_FOR:
               printf("(for ");
               printf("(");
               print_expr(stmt->for_stmt.expr_init);
               printf(")");
               printf("(");
               print_expr(stmt->for_stmt.expr_cond);
               printf(")");
               printf("(");
               print_expr(stmt->for_stmt.expr_update);
               printf(")");
               //printf("\n");
               //printf("(");
               print_stmt_block(stmt->block,indent++);
               printf(")");
               indent--;
               break;
          case STMT_WHILE:
               printf("( while ");
               print_expr(stmt->while_stmt.expr_cond);
               print_stmt_block(stmt->block,indent++);
               printf(")");
               indent--;
               break;
          case STMT_DO_WHILE:
               printf("( do ");
               print_stmt_block(stmt->block,indent++);
               printf(") while (");
               print_expr(stmt->while_stmt.expr_cond);
               printf(")"); 
               indent--;
               break;   
          case STMT_AUTO_ASSIGN:
               printf("( %s := ",stmt->auto_assign_stmt.name);
               print_expr( stmt->auto_assign_stmt.rhs); 
               printf(")");
               break;
          case STMT_OP_ASSIGN:
               printf("( ");
               print_expr(stmt->op_assign_stmt.lhs);
               printf(" = " );
               print_expr(stmt->op_assign_stmt.rhs);
               printf(" )");
               break;
          case STMT_RETURN:
               printf("( return ");
               print_expr(stmt->return_stmt.return_expr);
               printf(")");
               break;
               
//typedef struct Case {
//     Expr *expr; // buffer to hold a list of expressions
//     StmtBlock case_block;
//} Case;
//
//typedef struct stmt_switch_def{
//     Expr *expr;
//     size_t num_cases;
//    BUF( Case *cases;) // buffer to hold all the case statements;
//} stmt_switch_def;
          case STMT_SWITCH:
               {
                    stmt_switch_def tmp = stmt->switch_stmt;
                    printf("(switch ");
                    print_expr(tmp.expr);
                    
                    for ( Case *it = tmp.cases; it!= tmp.cases + tmp.num_cases; it++){
                         
                         printf("\n(case ");
                         print_expr( (it)->case_expr );
                         print_stmt_block( (it)->case_block, indent++ );
                         indent--;
                         printf("\nbreak)");
                    }
                    printf("\n(default ");
                    print_stmt_block(  tmp.default_block, indent++ );
                    indent--;
                    printf("))");
                    break;
               }
          default:
               assert(0);
               break;
     }
}

void print_decl(Decl *decl){
     switch ( decl->kind ){
          case DECL_ENUM:{
               enum_def new = decl->enum_decl;
               printf("(enum  %s ", decl->name);
               for ( enum_item **it = new.enum_items; it != new.enum_items + new.num_enum_items ; it++){
                    printf("\n\t");
                    printf("%s", (*it)->name );
                    if ( (*it)->expr != NULL ){
                         printf(":");
                         print_expr( (*it)->expr );
                    }
               }
               printf("\n)"); 
               break;
          }
          case DECL_STRUCT:
          case DECL_UNION:
               {
                    printf("(%s %s ",(decl->kind == DECL_STRUCT)?"struct":"union", decl->name);
                    aggregate_def tmp = decl->aggregate_decl;
                    for ( aggregate_item **it = tmp.aggregate_items; it != tmp.aggregate_items + tmp.num_aggregate_items;it++){
                         printf("\n\t");
                         print_name_list( (*it)->name_list, (*it)->num_names );
                         printf(":");
                         print_type( (*it)->type );
                    }
                    printf("\n)");
                    break;
               }
          case DECL_VAR:
               {
                    printf("(var %s",decl->name );
                    if ( decl->var_decl.type != NULL ){
                         printf(":");
                         print_type(decl->var_decl.type);
                    } else {
                         printf(":nil");
                    }
                    if ( decl->var_decl.expr != NULL ){
                        printf(" ");
                        print_expr(decl->var_decl.expr); 
                    }
                    printf(" )");
                    break;
               }
          case DECL_CONST:
               printf("(const %s ",decl->name);
               print_expr(decl->const_decl.expr);
               printf(" )");
               break;
          case DECL_TYPEDEF:
               printf("(typedef %s ",decl->name);
               print_type(decl->typedef_decl.type);
               printf(" )");
               break;
          case DECL_FUNC:
               {
                    func_def tmp = decl->func_decl;
                    printf("(func %s ( ", decl->name );
                    for (func_param **it = tmp.param_list; it!=tmp.param_list+tmp.num_params; it++ ){
                         printf("(%s:", (*it)->name );
                         print_type((*it)->type);
                         printf(") ");
                    }
                    printf(") : ");
                    print_type(tmp.ret_type);
                    printf(" )");
                    break;
               }
          default:
               assert(0);
               break;           
     }
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
               printf("(func (");
               for ( TypeSpec **it = fn.args; it != fn.args + fn.num_args ; it++){
                    printf(" ");
                    print_type(*it);
               }
               printf("):");
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
               print_expr(expr->field_expr.operand);
               printf(" %s)",expr->field_expr.field_name);
               break;
          case EXPR_INDEX:
               printf("(index ");
               print_expr(expr->array_expr.operand);
               printf(" ");
               print_expr(expr->array_expr.index);
               printf(")");
               break;
          case EXPR_CAST:
               printf("(cast ");
               print_type(expr->cast_expr.cast_type);
               printf(" ");
               print_expr(expr->cast_expr.expr);
               printf(")");
               break;
          case EXPR_CALL:
               printf("(");
               print_expr(expr->func_call_expr.operand);
               for ( Expr **it = expr->func_call_expr.args; it != expr->func_call_expr.args + expr->func_call_expr.num_args; it++){
                    printf(" ");
                    print_expr(*it);
               }
               printf(")");
               break;
          case EXPR_BINARY:
               printf("(%s ", operations[expr->op]);
               print_expr(expr->binary_expr.left);
               printf(" ");
               print_expr(expr->binary_expr.right);
               printf(")");
               break;
          case EXPR_UNARY:
               printf("(%c ", expr->op);
               print_expr(expr->unary_expr.operand);
               printf(")");
               break;
          case EXPR_TERNARY:
               printf("(if ");
               print_expr(expr->ternary_expr.cond_expr);
               printf(" ");
               print_expr(expr->ternary_expr.then_expr);
               printf(" ");
               print_expr(expr->ternary_expr.else_expr);
               printf(")");
               break;
          case EXPR_COMPOUND:
               printf("( ");
               print_type(expr->compound_expr.type);
               printf(" (");
               for ( Expr **it = expr->compound_expr.args; it != expr->compound_expr.args + expr->compound_expr.num_args; it++ ){
                    printf(" ");
                    print_expr(*it); 
               }
               
               printf(") )");
               break; 
          default:
               assert(0);
               break;

     }
}



// Declaration constructors [declcon]
Decl *decl_new( DeclKind kind, const char *name ){
     Decl *new_decl = xcalloc(1,sizeof(Decl));
     new_decl->kind = kind;
     new_decl->name = name;
     return new_decl;
}

Decl *decl_enum(const char *name, size_t num_enum_items, enum_item **item_list){
     Decl *new_decl = decl_new(DECL_ENUM, name);
     new_decl->enum_decl.num_enum_items = num_enum_items;
     new_decl->enum_decl.enum_items = item_list;
     return new_decl;
}

Decl *decl_aggregate( DeclKind kind, const char *name, size_t num_aggregate_items, aggregate_item **item_list ){
     Decl *new = decl_new(kind,name);
     new->aggregate_decl.num_aggregate_items = num_aggregate_items;
     new->aggregate_decl.aggregate_items = item_list;
     return new;
}

Decl *decl_var(const char *name, TypeSpec *type, Expr *expr ){
     Decl *new = decl_new(DECL_VAR,name);
     new->var_decl.type = type;
     new->var_decl.expr = expr;
     return new;
}

Decl *decl_const( const char *name, Expr *expr ){
     Decl *new = decl_new(DECL_CONST,name);
     new->const_decl.expr = expr;
     return new;
}

Decl *decl_typedef( const char *name , TypeSpec *type){
     Decl *new = decl_new(DECL_TYPEDEF, name );
     new->typedef_decl.type = type;
     return new;
}

Decl *decl_func( const char *name, TypeSpec *ret_type, func_param **param_list, size_t num_params){
     Decl *new = decl_new(DECL_FUNC, name );
     new->func_decl.ret_type = ret_type;
     new->func_decl.num_params = num_params;
     new->func_decl.param_list = param_list;
     return new;
} 


enum_item *new_enum( const char *name, Expr *expr , TypeSpec *type ){
     enum_item *new = xcalloc(1,sizeof(enum_item));
     new->name = name;
     new->expr = expr;
     new->type = type;
     return new;
}

aggregate_item *new_aggregate( const char **name_list,size_t num_names, TypeSpec *type ){
     aggregate_item *new = xcalloc(1,sizeof(aggregate_item) );
     new->name_list = name_list;
     new->num_names = num_names;
     new->type = type;
     return new;
}

func_param *new_func_param( const char *name, TypeSpec *type ){
     func_param *new = xcalloc(1,sizeof(func_param) );
     new->name = name;
     new->type = type;
     return new;
}

// List Constructors [listcon]

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

const char **name_list( size_t count, ... ){
     va_list names;
     va_start(names,count);
     const char **list = NULL ; 
     for ( int i = 0; i < count ; i++ ){
          buff_push(list,va_arg(names,char*) );
     }
     va_end(names);
     return list;
}

Stmt **stmt_list(size_t num_stmts, ... ){
     va_list stmts;
     va_start(stmts,num_stmts);
     Stmt **stmt_list = NULL;
     for ( int i = 0; i < num_stmts; i++ ){
          buff_push(stmt_list,va_arg(stmts,Stmt*));
     }
     va_end( stmts );
     return stmt_list;
}

Case *case_list( size_t num_cases, ... ){
     va_list cases;
     va_start(cases,num_cases);
     Case *tmp = NULL;
     for ( int i = 0; i < num_cases; i++ ){
          buff_push(tmp,va_arg(cases,Case));
     }
     va_end(cases);
     return tmp;
}

// Statement Constructors [stmtcon]
//typedef struct Elseif {
//     Expr *cond;
//     StmtBlock elseif_block;
//} Elseif;
//
//typedef struct stmt_if_def {
//     Elseif **elseifs; // list of pointers to else_if blocks
//     StmtBlock else_block;
//} stmt_if_def;
//

StmtBlock new_block(size_t num_stmts, Stmt **stmts){
    return (StmtBlock){num_stmts,stmts};
     
}

Stmt *new_stmt(StmtKind kind, StmtBlock block ){
     Stmt *new = xcalloc(1,sizeof(Stmt));
     new->kind = kind;
     new->block = block;
    return new; 
}


Elseif new_elif( Expr *cond, StmtBlock block){
     return (Elseif){cond,block};
}

Elseif *elseif_list(size_t num, ... ){
     va_list elifs;
     va_start(elifs,num);
     Elseif *new = NULL;
     for ( int i = 0; i < num ; i++ ){
          buff_push(new,va_arg(elifs,Elseif));
     }
     va_end(elifs);
     return new;
}

Stmt *stmt_if( Expr *condition,StmtBlock if_block,size_t num_elseifs, Elseif *elseifs, StmtBlock else_block ){
     Stmt *new= new_stmt(STMT_IF,if_block);
     new->if_stmt.cond = condition;
     new->if_stmt.elseifs = elseifs;
     new->if_stmt.num_elseifs = num_elseifs;
     new->if_stmt.else_block = else_block;
     return  new;
}

//typedef enum StmtKind {
//     STMT_NONE,
//     STMT_AUTO_ASSIGN,
//     STMT_ASSIGN,
//     STMT_FOR,
//     STMT_WHILE,
//     STMT_DO_WHILE,
//     STMT_IF,
//     STMT_SWITCH,
//     STMT_RETURN,
//     STMT_BREAK,
//     STMT_CONTINUE
//} StmtKind;

Stmt *stmt_auto_assign(const char *name, Expr *expr){
     Stmt *new = new_stmt(STMT_AUTO_ASSIGN,(StmtBlock){0,NULL});
     new->auto_assign_stmt.rhs = expr;
     new->auto_assign_stmt.name = name;
     return new;
}
Stmt *stmt_op_assign(Expr *lhs, TokenKind op, Expr *rhs){
     Stmt *new = new_stmt(STMT_OP_ASSIGN,(StmtBlock){0,NULL});
     new->op_assign_stmt.lhs = lhs;
     new->op_assign_stmt.op = op;
     new->op_assign_stmt.rhs = rhs;
     return new;
}
Stmt *stmt_for(Expr *init, Expr *cond, Expr *update, StmtBlock block){
     Stmt *new = new_stmt(STMT_FOR,block);
     new->for_stmt.expr_init = init;
     new->for_stmt.expr_cond = cond;
     new->for_stmt.expr_update = update;
     return new;
}
Stmt *stmt_while(Expr *cond, StmtBlock block){
     Stmt *new = new_stmt(STMT_WHILE,block);
     new->while_stmt.expr_cond = cond;
     return new;
}

Stmt *stmt_do_while(Expr *cond, StmtBlock block){
     Stmt *new = new_stmt(STMT_DO_WHILE,block);
     new->while_stmt.expr_cond = cond;
     return new;
}

Case new_case( Expr *expr, StmtBlock block ){
     return (Case){expr,block};
}

Stmt *stmt_switch(Expr *expr, size_t num_cases, Case *cases, StmtBlock default_block){
     Stmt *new = new_stmt(STMT_SWITCH,default_block);
     new->switch_stmt.expr = expr;
     new->switch_stmt.num_cases = num_cases;
     new->switch_stmt.cases = cases;
     new->switch_stmt.default_block = default_block;
     return new;
}
Stmt *stmt_return(Expr *expr){
     Stmt *new = new_stmt(STMT_RETURN, (StmtBlock){0,NULL});
     new->return_stmt.return_expr =  expr;
     return new;
}


Stmt *stmt_break(void){
     return new_stmt(STMT_BREAK, (StmtBlock){0,NULL } );
}

Stmt *stmt_continue(void){
     return new_stmt(STMT_CONTINUE,(StmtBlock){0,NULL});
}



// Test Function [tstfunc]
//

void ast_decl_test(){
     enum_item *enum_item_list[] = { 
          new_enum("ABC",expr_int(2),NULL),
          new_enum("def",expr_int(3),NULL),
          new_enum("ghi",NULL,NULL)
     };
     
     aggregate_item *agg_list_tmp[] = {
          new_aggregate( name_list(2,"length","age"),2, type_name("uint") ),
          new_aggregate( name_list(1,"height"),1,type_name("float")),
          new_aggregate(name_list(1,"integer_pointer"),1,type_pointer(type_name("int")))
     };

     func_param *func_param_list[] = {
         new_func_param("a",type_name("int")),
         new_func_param("x",type_pointer(type_name("int"))) 
     };

     
    assert( &enum_item_list[1] == &enum_item_list[0]+1 ); 

     Decl *decl_list[] = {
          decl_enum("Alphabet",3,enum_item_list),
          decl_aggregate(DECL_STRUCT,"Person",sizeof(agg_list_tmp)/sizeof(aggregate_item *),agg_list_tmp),
          decl_aggregate(DECL_UNION,"Person",sizeof(agg_list_tmp)/sizeof(aggregate_item *),agg_list_tmp),
          decl_var("x",type_name("int"),expr_int(3)),
          decl_var("x",NULL,expr_int(2)),
          decl_var("x",type_pointer(type_name("int")),NULL), 
          decl_const("x",expr_binary('+',expr_int(2),expr_int(3))),
          decl_typedef("board",type_name("cells")),
          decl_func("foo", type_name("int"), func_param_list ,sizeof(func_param_list)/sizeof(func_param *) )
     };

     for ( Decl **it = decl_list; it != decl_list + sizeof(decl_list)/sizeof(Decl *) ; it++ ){
          print_decl( *it );
          putchar('\n');
     }
}


void ast_stmt_test(){

     Stmt **stmt1 = stmt_list(3,stmt_break(),stmt_op_assign(expr_name("i"),'=',expr_name("x")),stmt_while(expr_binary(EQ,expr_name("ab"),expr_int(1)),new_block(1,stmt_list(1,stmt_continue()))));
     

     Stmt **stmt2 = stmt_list(2,stmt_return(expr_int(1)), stmt_for(expr_binary('=',expr_name("i"),expr_int(1)),expr_binary(LTEQ,expr_name("i"),expr_int(10)),expr_binary('+',expr_name("i"),expr_int(1)),new_block(1,stmt_list(1,stmt_continue()))));     
     StmtBlock block_test1 = new_block(3,stmt1 );

     StmtBlock block_test2 = new_block(2,stmt2);
     Elseif *elifs = elseif_list(1,new_elif(expr_int(1),block_test1));

     Case *cases = case_list(2, new_case(expr_int(1),block_test1), new_case(expr_int(2),block_test2));
     
     Stmt *stmt_list[] = {
          stmt_if(expr_binary(LSHIFT_ASSIGN,expr_int(1),expr_int(2)),block_test1,1,elifs,block_test1),
          stmt_while(expr_binary(LTEQ,expr_name("abc"),expr_int(12)), block_test1),
          stmt_do_while(expr_binary(LTEQ,expr_name("c"),expr_int(15)), block_test2),
          stmt_switch(expr_name("a"),2,cases,block_test2),
          stmt_auto_assign("a",expr_int(123))
     };
//     print_stmt_block(block_test);
//     print_stmt(stmt_if(expr_binary(LSHIFT_ASSIGN,expr_int(1),expr_int(2)),block_test,0,NULL,block_test));
     putchar('\n');

     for ( Stmt **it = stmt_list; it != stmt_list + sizeof(stmt_list)/sizeof(Stmt *); it++ ){
          print_stmt(*it);
          putchar('\n');
     }
     
}


void ast_expr_test(){
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
          expr_index(expr_name("expr_list"),expr_int(34)),
          expr_compound( type_name("Vector"),exps , 3)
     };
     Expr *new_expr = expr_int( 123 );
     assert( new_expr->kind == EXPR_INT && new_expr->int_val == 123 );

          putchar('\n');
     for ( Expr **it = expr_list; it != expr_list + sizeof(expr_list)/sizeof(Expr *) ; it++ ){
          print_expr( *it );
          putchar('\n');
     }
}
void ast_test(){
     ast_decl_test();
     ast_expr_test();
     ast_stmt_test();     

}
