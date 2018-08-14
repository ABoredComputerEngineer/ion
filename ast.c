#include "ast.h"
// Expressions constructors [exprcon]
Expr *expr_int( uint64_t int_val );
Expr *expr_float( double float_val);
Expr *expr_str( const char *str_val);
Expr *expr_name( const char *name);
Expr *expr_unary( TokenKind op, Expr *expr);
Expr *expr_binary( TokenKind op, Expr *left, Expr *right);
Expr *expr_ternary( Expr *cond, Expr *then_expr, Expr *else_expr);
Expr *expr_cast( TypeSpec *type, Expr *expr);
Expr *expr_call(Expr *operand, Expr **args, size_t num_args);
Expr *expr_index(Expr *operand, Expr *index);
Expr *expr_compound( TypeSpec *type, Expr **args, size_t num_args );

Arena arena_ast; // Memory to store all ast related objects

void *ast_alloc( size_t size ){
     void *ptr = arena_alloc(&arena_ast,size);
     memset(ptr,0,size);
     return ptr;
}

// TypeSpec constructors [typecon]

TypeSpec *type_alloc(TypeSpecKind kind){
     TypeSpec *new = ast_alloc(sizeof(TypeSpec));
//     TypeSpec *new = xcalloc(1,sizeof(TypeSpec));
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
     Expr *new_expr = ast_alloc(sizeof(Expr));
//     Expr *new_expr = xcalloc(1,sizeof(Expr));
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
Expr *expr_sizeof_expr(Expr *expr){
     Expr *new= expr_new(EXPR_SIZEOF_EXPR);
     new->sizeof_expr = expr;
     return new;
}

Expr *expr_sizeof_type(TypeSpec *type){
     Expr *new= expr_new(EXPR_SIZEOF_TYPE);
     new->sizeof_type = type;
     return new;
}

// Print Functios [prnfunc]
#define print_newline  printf("\n%.*s",indent,indent_string),assert(indent>=0)
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
     [ TOKEN_LSHIFT] = "<<",
     [ TOKEN_RSHIFT] = ">>",
     [ TOKEN_EQ ] = "==",
     [ TOKEN_NOTEQ ] = "!=",
     [ TOKEN_LTEQ ] = "<=",
     [ TOKEN_GTEQ ] = ">=",
     [ TOKEN_AND ] = "&&",
     [ TOKEN_OR ] = "||",
     [ TOKEN_ADD_ASSIGN ] = "+=",
     [ TOKEN_MUL_ASSIGN ] = "*=",
     [ TOKEN_SUB_ASSIGN ] = "-+",
     [ TOKEN_DIV_ASSIGN ] = "/=",
     [ TOKEN_MOD_ASSIGN ] = "%=",
     [ TOKEN_AND_ASSIGN ] = "&=",
     [ TOKEN_OR_ASSIGN ] = "|=",
     [ TOKEN_XOR_ASSIGN ] = "^=",
     [ TOKEN_LSHIFT_ASSIGN ] = "<<=",
     [ TOKEN_RSHIFT_ASSIGN ] = ">>=",
     [ TOKEN_COLON_ASSIGN ] = ":=",
     [ TOKEN_INC ] = "++",
     [ TOKEN_DEC ] = "--",
     [ TOKEN_ASSIGN ] = "=",
     ['+'] = "+",
     ['-'] = "-",
     [ '*' ] = "*",
     [ '/' ] = "/",
     [ '%' ] = "%" ,
     [ '~' ] = "~",
     [ '&' ] = "&",
     [ '^' ] = "^",
     [ '|' ] = "|",
     [ '<' ] = "<",
     [ '>' ] = ">",
     //[ '<' ] = "<",
     //[ '<' ] = "<",
};

void print_stmt_block(StmtBlock block,int indent){
     for ( Stmt **stmt = block.stmts; stmt != block.stmts + block.num_stmts ; stmt++ ){
//          putchar('\n');
          print_newline;
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
//                    printf("\n%.*s",indent,indent_string);
                    //indent++;
                    print_newline;
                    printf("(then ");
                    print_stmt_block(stmt->if_stmt.if_block,++indent);
                    printf(")");
                    indent--;
                    if ( tmp.elseifs != NULL ){
                         for ( Elseif *it = tmp.elseifs; it != tmp.elseifs + tmp.num_elseifs ; it++){
                              print_newline;
                              printf("( elseif ");
                              print_expr( (it)->cond);
                              print_newline;
                              printf("(then ");
                              print_stmt_block( (it)->elseif_block, ++indent );
                              indent--;
                              printf(")");
                         }
                    }
                    if ( tmp.else_block.num_stmts != 0 ){
                         print_newline;
                         printf("(else ");
                         print_stmt_block(tmp.else_block,++indent);
                         printf(")");
                         indent--;
                    }
                    //indent-=1;
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
               if ( stmt->for_stmt.stmt_init ){
                    print_stmt(stmt->for_stmt.stmt_init);
               }
               printf(")");
               printf("(");
               if ( stmt->for_stmt.expr_cond ){
               print_expr(stmt->for_stmt.expr_cond);
               }
               printf(")");
               printf("(");
               if ( stmt->for_stmt.stmt_update ){
               print_stmt(stmt->for_stmt.stmt_update);
               }
               printf(")");
               //printf("\n");
               //printf("(");
               print_stmt_block(stmt->for_stmt.block,++indent);
               printf(")");
               indent--;
               break;
          case STMT_WHILE:
               printf("( while ");
               print_expr(stmt->while_stmt.expr_cond);
               print_stmt_block(stmt->while_stmt.block,++indent);
               printf(")");
               indent--;
               break;
          case STMT_DO_WHILE:
               printf("( do ");
               print_stmt_block(stmt->while_stmt.block,++indent);
               printf(") while (");
               print_expr(stmt->while_stmt.expr_cond);
               printf(")"); 
               indent--;
               break;   
          case STMT_INIT:
               printf("( %s := ",stmt->assign_stmt.name);
               print_expr( stmt->assign_stmt.rhs); 
               printf(")");
               break;
        //  case STMT_OP_ASSIGN:
        //       printf("( ");
        //       print_expr(stmt->op_assign_stmt.lhs);
        //       printf(" = " );
        //       print_expr(stmt->op_assign_stmt.rhs);
        //       printf(" )");
        //       break;
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
                             indent++; 
                              print_newline;
                    
                    for ( Case *it = tmp.cases; it!= tmp.cases + tmp.num_cases; it++){
                         if ( !(it->isdefault) ){    
                              printf("(case ");
                              for ( Expr **ie = it->expr_list; ie!=it->expr_list+it->num_expr; ie++){
                                   print_expr( *ie );
                                   printf(" ");
                              }
                         } else {
                              //print_newline;
                              printf("(default ");
                         }
                         print_stmt_block( (it)->case_block, ++indent );
                         printf(")");
                         indent--;
                         print_newline;
                         //indent-=1;
                    }
                    indent--;
                    print_newline;
                    printf(")");
                    break;
               }

          case STMT_EXPR:
               printf("( ");
               print_expr(stmt->expr_stmt);
               printf(")");
               break;
          default:
               assert(0);
               break;
     }
}

void print_decl(Decl *decl){ // TODO : Add print StmtBlock to func declarations
     switch ( decl->kind ){
          case DECL_ENUM:{
               enum_def new = decl->enum_decl;
               printf("(enum  %s ", decl->name);
               indent++;
               for ( enum_item *it = new.enum_items; it != new.enum_items + new.num_enum_items ; it++){
                    print_newline;
                    printf("%s", (it)->name );
                    if ( (it)->expr != NULL ){
                         printf(":");
                         print_expr( (it)->expr );
                    }
               }
               indent--;
               printf("\n)"); 
               break;
          }
          case DECL_STRUCT:
          case DECL_UNION:
               {
                    printf("(%s %s ",(decl->kind == DECL_STRUCT)?"struct":"union", decl->name);
                    aggregate_def tmp = decl->aggregate_decl;
                         indent++;
                    for ( aggregate_item *it = tmp.aggregate_items; it != tmp.aggregate_items + tmp.num_aggregate_items;it++){
                       //  printf("\n\t");
                         print_newline;
                         print_name_list( (it)->name_list, (it)->num_names );
                         printf(":");
                         print_type( (it)->type );
                    }
                    indent--;
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
                    for (func_param *it = tmp.param_list; it!=tmp.param_list+tmp.num_params; it++ ){
                         printf("(%s:", (it)->name );
                         print_type((it)->type);
                         printf(") ");
                    }
                    printf(")");
                    if ( tmp.ret_type ) { // type is not NULL i.e there is a return type
                         printf(" : ");
                         print_type(tmp.ret_type);
                    }
                    
                    printf(" )");
                    printf("(");
                    print_stmt_block(tmp.block,++indent);
                    indent--;
                    print_newline;
                    printf(")");
                    break;
               }
          default:
               assert(0);
               break;           
     }
}
void print_type(TypeSpec *type){
     if ( type == NULL ){
          return;
     }
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
               func_typespec fn = type->func_decl;
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
               printf("(");
               for ( Expr **it = expr->func_call_expr.args; it != expr->func_call_expr.args + expr->func_call_expr.num_args - 1; it++){
                    print_expr(*it);
                    printf(",");
               }
               print_expr( *(expr->func_call_expr.args+expr->func_call_expr.num_args - 1) );
               printf(")");
               printf(") ");
               break;
          case EXPR_BINARY:
               printf("(%s ", operations[expr->op]);
               print_expr(expr->binary_expr.left);
               printf(" ");
               print_expr(expr->binary_expr.right);
               printf(")");
               break;
          case EXPR_UNARY:
               printf("(%s ", operations[expr->op]);
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
               for ( Expr **it = expr->compound_expr.args; it != expr->compound_expr.args + expr->compound_expr.num_args - 1; it++ ){
                    print_expr(*it); 
                    printf(" ,");
               }
               print_expr( *( expr->compound_expr.args + expr->compound_expr.num_args - 1 ) ); 
               printf(") )");
               break; 
          case EXPR_SIZEOF_EXPR:
               printf("(sizeof ");
               print_expr(expr->sizeof_expr);
               printf(")");
               break;
          case EXPR_SIZEOF_TYPE:
               printf("(sizeof ");
               print_type(expr->sizeof_type);
               printf(")");
               break;
          default:
               assert(0);
               break;

     }
}



// Declaration constructors [declcon]
Decl *decl_new( DeclKind kind, const char *name ){
//     Decl *new_decl = xcalloc(1,sizeof(Decl));
     Decl *new_decl = ast_alloc(sizeof(Decl));
     new_decl->kind = kind;
     new_decl->name = name;
     return new_decl;
}

Decl *decl_enum(const char *name, size_t num_enum_items, enum_item *item_list){
     Decl *new_decl = decl_new(DECL_ENUM, name);
     new_decl->enum_decl.num_enum_items = num_enum_items;
     new_decl->enum_decl.enum_items = item_list;
     return new_decl;
}

Decl *decl_aggregate( DeclKind kind, const char *name, size_t num_aggregate_items, aggregate_item *item_list ){
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

//TODO : Add StmtBlock
Decl *decl_func( const char *name, TypeSpec *ret_type, func_param *param_list, size_t num_params,StmtBlock block){
     Decl *new = decl_new(DECL_FUNC, name );
     new->func_decl.ret_type = ret_type;
     new->func_decl.num_params = num_params;
     new->func_decl.param_list = param_list;
     new->func_decl.block = block;
     return new;
} 


enum_item new_enum( const char *name, Expr *expr ){
//     enum_item *new = xcalloc(1,sizeof(enum_item));
//     new->name = name;
//     new->expr = expr;
//     new->type = type;
     return (enum_item){name,expr}; 
}

aggregate_item new_aggregate( const char **name_list,size_t num_names, TypeSpec *type ){
 //    aggregate_item *new = xcalloc(1,sizeof(aggregate_item) );
 //    new->name_list = name_list;
 //    new->num_names = num_names;
 //    new->type = type;
     return (aggregate_item){name_list,num_names,type}; 
}

func_param new_func_param( const char *name, TypeSpec *type ){
     return (func_param){name,type};
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

Stmt *new_stmt(StmtKind kind){
//     Stmt *new = xcalloc(1,sizeof(Stmt));
     Stmt *new = ast_alloc(sizeof(Stmt));
     new->kind = kind;
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
     Stmt *new= new_stmt(STMT_IF);
     new->if_stmt.if_block = if_block;
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
Stmt *stmt_expr(Expr *);
Stmt *stmt_assign(TokenKind op, Expr *lhs, Expr *rhs){
     if ( op == TOKEN_INC || op == TOKEN_DEC ){
          return stmt_expr(expr_unary(op,lhs));
     } else {
          return stmt_expr(expr_binary(op,lhs,rhs));
     }
}
Stmt *stmt_init(const char *name, Expr *expr){
     Stmt *new = new_stmt(STMT_INIT);
     new->init_stmt.rhs = expr;
     new->init_stmt.name = name;
     return new;
}
Stmt *stmt_for(Stmt *init, Expr *cond, Stmt *update, StmtBlock block){
     Stmt *new = new_stmt(STMT_FOR);
     new->for_stmt.block = block;
     new->for_stmt.stmt_init = init;
     new->for_stmt.expr_cond = cond;
     new->for_stmt.stmt_update = update;
     return new;
}
Stmt *stmt_while(Expr *cond, StmtBlock block){
     Stmt *new = new_stmt(STMT_WHILE);
     new->while_stmt.block = block;
     new->while_stmt.expr_cond = cond;
     return new;
}

Stmt *stmt_do_while(Expr *cond, StmtBlock block){
     Stmt *new = new_stmt(STMT_DO_WHILE);
     new->while_stmt.block = block;
     new->while_stmt.expr_cond = cond;
     return new;
}

Case new_case( Expr **expr,size_t num_expr, bool isdefault, StmtBlock block ){
     return (Case){expr,num_expr,isdefault,block};
}

Stmt *stmt_switch(Expr *expr, size_t num_cases, Case *cases){
     Stmt *new = new_stmt(STMT_SWITCH);
     new->switch_stmt.expr = expr;
     new->switch_stmt.num_cases = num_cases;
     new->switch_stmt.cases = cases;
     return new;
}
Stmt *stmt_return(Expr *expr){
     Stmt *new = new_stmt(STMT_RETURN);
     new->return_stmt.return_expr =  expr;
     return new;
}


Stmt *stmt_break(void){
     return new_stmt(STMT_BREAK);
}

Stmt *stmt_continue(void){
     return new_stmt(STMT_CONTINUE);
}

Stmt *stmt_expr(Expr *expr){
     Stmt *new = new_stmt(STMT_EXPR);
     new->expr_stmt = expr;
     return new;

}

// Test Function [tstfunc]
//

void ast_decl_test(){
     Stmt **stmt1 = stmt_list(3,stmt_break(),stmt_expr(expr_name("i")),stmt_while(expr_binary(TOKEN_EQ,expr_name("ab"),expr_int(1)),new_block(1,stmt_list(1,stmt_continue()))));
     
     StmtBlock block_test1 = new_block(3,stmt1 );
     enum_item enum_item_list[] = { 
          new_enum("ABC",expr_int(2)),
          new_enum("def",expr_int(3)),
          new_enum("ghi",NULL)
     };
     
     aggregate_item agg_list_tmp[] = {
          new_aggregate( name_list(2,"length","age"),2, type_name("uint") ),
          new_aggregate( name_list(1,"height"),1,type_name("float")),
          new_aggregate(name_list(1,"integer_pointer"),1,type_pointer(type_name("int")))
     };

     func_param func_param_list[] = {
         new_func_param("a",type_name("int")),
         new_func_param("x",type_pointer(type_name("int"))) 
     };

     
    assert( &enum_item_list[1] == &enum_item_list[0]+1 ); 

     Decl *decl_list[] = {
          decl_enum("Alphabet",3,enum_item_list),
          decl_aggregate(DECL_STRUCT,"Person",sizeof(agg_list_tmp)/sizeof(aggregate_item ),agg_list_tmp),
          decl_aggregate(DECL_UNION,"Person",sizeof(agg_list_tmp)/sizeof(aggregate_item),agg_list_tmp),
          decl_var("x",type_name("int"),expr_int(3)),
          decl_var("x",NULL,expr_int(2)),
          decl_var("x",type_pointer(type_name("int")),NULL), 
          decl_const("x",expr_binary('+',expr_int(2),expr_int(3))),
          decl_typedef("board",type_name("cells")),
          decl_func("foo", type_name("int"), func_param_list ,sizeof(func_param_list)/sizeof(func_param),block_test1 )
     };

     for ( Decl **it = decl_list; it != decl_list + sizeof(decl_list)/sizeof(Decl *) ; it++ ){
          print_decl( *it );
          putchar('\n');
     }
}


void ast_stmt_test(){

     Stmt **stmt1 = stmt_list(3,stmt_break(),stmt_expr(expr_name("i")),stmt_while(expr_binary(TOKEN_EQ,expr_name("ab"),expr_int(1)),new_block(1,stmt_list(1,stmt_continue()))));
     
     Stmt **stmt2 = stmt_list(3, stmt_return(expr_name("true")),stmt_expr(expr_binary(TOKEN_LTEQ,expr_name("a"),expr_int(3))),stmt_continue());
     StmtBlock block_test1 = new_block(3,stmt1 );

     StmtBlock block_test2 = new_block(2,stmt2);
     Elseif *elifs = elseif_list(1,new_elif(expr_int(1),block_test1));

     Case *cases = case_list(2, new_case(expr_list(2,expr_int(1),expr_name("a")),2,false,block_test1), new_case(expr_list(1,expr_int(2)),1,true,block_test2));
     
     Stmt *stmt_list[] = {
          stmt_if(expr_binary(TOKEN_LSHIFT_ASSIGN,expr_int(1),expr_int(2)),block_test1,1,elifs,block_test1),
          stmt_while(expr_binary(TOKEN_LTEQ,expr_name("abc"),expr_int(12)), block_test1),
          stmt_do_while(expr_binary(TOKEN_LTEQ,expr_name("c"),expr_int(15)), block_test2),
          stmt_switch(expr_name("a"),2,cases),
          stmt_init("a",expr_int(123))
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
