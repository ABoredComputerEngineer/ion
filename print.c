#include "ast.h"
char *print_buffer;
bool use_buff_print ;
#define printf(...) ( use_buff_print ? (void)buff_printf(print_buffer,__VA_ARGS__) : (void)printf(__VA_ARGS__) )

void flush_print_buff(FILE *file){
     if ( print_buffer ){
          if ( file ){
               fputs(print_buffer,file);
          }
          buff_clear(print_buffer);
     }
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
     [ TOKEN_ADD ] = "+",
     [ TOKEN_SUB ] = "-",
     [ TOKEN_MUL ] = "*",
     [ TOKEN_DIV ] = "/",
     [ TOKEN_MOD ] = "%" ,
     [ TOKEN_COMPLEMENT ] = "~",
     [ TOKEN_BAND ] = "&",
     [ TOKEN_XOR ] = "^",
     [ TOKEN_BOR ] = "|",
     [ TOKEN_LT ] = "<",
     [ TOKEN_GT ] = ">",
     [ TOKEN_NOT ] = "!",
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
               if ( stmt->return_stmt.return_expr != NULL ){
                    print_expr(stmt->return_stmt.return_expr);
               }
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
          case STMT_DECL:
               print_decl(stmt->decl_stmt);
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
               print_type( type->array.base_type );
               printf("[");
               print_expr(type->array.size);
               printf("])");
               break;
          case TYPESPEC_POINTER:
               printf("(ptr ");
               print_type(type->ptr.base_type);
               printf(")");
               break;
          case TYPESPEC_FUNC:{
               func_typespec fn = type->func;
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
               printf("(%s ", operations[expr->binary_expr.op]);
               print_expr(expr->binary_expr.left);
               printf(" ");
               print_expr(expr->binary_expr.right);
               printf(")");
               break;
          case EXPR_UNARY:
               printf("(%s ", operations[expr->binary_expr.op]);
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
               printf("(");
               for ( CompoundField *it = expr->compound_expr.fields; it != expr->compound_expr.fields + expr->compound_expr.num_args; it++ ){
                    if ( it->kind != FIELD_NONE ){
                         printf("(");
                         print_expr(it->field_expr);
                         printf(") = ");
                    }
                    print_expr(it->expr);
                    printf("%c",it==expr->compound_expr.fields + expr->compound_expr.num_args-1?' ':','); 
               }
//               for ( Expr **it = expr->compound_expr.args; it != expr->compound_expr.args + expr->compound_expr.num_args - 1; it++ ){
//                    print_expr(*it); 
//                    printf(" ,");
//               }
//               print_expr( *( expr->compound_expr.args + expr->compound_expr.num_args - 1 ) ); 
               printf("))");
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
