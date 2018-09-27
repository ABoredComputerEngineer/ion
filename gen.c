Arena gen_arena;
char *gen_buff = NULL;
bool use_gen_buff = false;
#ifdef printf
     #undef printf
#endif
#define printf(...) ( (use_gen_buff)?(void)buff_printf(gen_buff,__VA_ARGS__):(void)printf(__VA_ARGS__))

//Indentation handling

#ifdef new_line
     #undef new_line
#endif

#define new_line printf("\n%.*s",indent,"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t")
extern int indent;
extern Sym **global_sym_list ;
extern Sym **ordered_syms ;
char *bprintf( const char *fmt,... ){
     va_list args;
     va_start(args,fmt);
     size_t len = 1 + vsnprintf(NULL,0,fmt,args);
     va_end(args);
     va_start(args,fmt);
     char *dest = arena_alloc(&gen_arena,len * sizeof( char ) );
     size_t new_len = 1 + vsnprintf(dest,len,fmt,args);
     assert(new_len <= len );
     va_end(args);
     return dest;
}





char *gen_expr(Expr *expr){
     extern char *type_to_cdecl(Type *,char *);
     switch (expr->kind){
          case EXPR_NAME:
               return bprintf("%s",expr->name);
               break;
          case EXPR_INT:
               return bprintf("%d",expr->int_val);
               break;
          case EXPR_FLOAT:
               return bprintf("%f",expr->float_val);
               break;
          case EXPR_UNARY:
               return bprintf("%s(%s)",operations[expr->unary_expr.op],gen_expr(expr->unary_expr.operand));
               break;
          case EXPR_BINARY:
               return bprintf("(%s)%s(%s)",gen_expr(expr->binary_expr.left),operations[expr->binary_expr.op],gen_expr(expr->binary_expr.right));
               break;
          case EXPR_STR:
               return bprintf("\"%s\"",expr->str_val);
          case EXPR_CAST:
               return bprintf("(%s)(%s)",type_to_cdecl(expr->cast_expr.resolved_type,""),gen_expr(expr->cast_expr.expr));
          case EXPR_CALL: {
               char *result = NULL;
               result  = bprintf("%s(",gen_expr(expr->func_call_expr.operand));
               for ( size_t i = 0; i < expr->func_call_expr.num_args ; i++ ){
                    result = bprintf("%s%s%s",result,i==0?"":",",gen_expr(expr->func_call_expr.args[i]));
               }
               result = bprintf("%s)",result);
               return result;
               break;
          }
          case EXPR_INDEX:
               return bprintf("%s[%s]",gen_expr(expr->array_expr.operand),gen_expr(expr->array_expr.index));
          case EXPR_FIELD:
               return bprintf("(%s).%s",gen_expr(expr->field_expr.operand),expr->field_expr.field_name);
          case EXPR_TERNARY:
               return bprintf("(%s)?(%s):(%s)",gen_expr(expr->ternary_expr.cond_expr)\
                         ,gen_expr(expr->ternary_expr.then_expr)\
                         ,gen_expr(expr->ternary_expr.else_expr));
          case EXPR_COMPOUND:{
               char *result = NULL;
               if ( expr->compound_expr.resolved_type->kind  != TYPE_ARRAY ){
                    result =  bprintf("(%s){",type_to_cdecl(expr->compound_expr.resolved_type,""));
               } else {
                    result =  bprintf("{",type_to_cdecl(expr->compound_expr.resolved_type,""));
               }
               for ( CompoundField *it = expr->compound_expr.fields;\
                         it != expr->compound_expr.fields + expr->compound_expr.num_args;\
                         it++ ){
                    if ( it->kind == FIELD_NONE ){
                         result = bprintf("%s%s%s",result,\
                                   (it == expr->compound_expr.fields)?"":",",\
                                   it->expr);
                    } else if ( it->kind == FIELD_INDEX ){
                         result = bprintf("%s%s[%s]=%s",result,\
                                  (it == expr->compound_expr.fields)?"":",",\
                                 gen_expr(it->field_expr),\
                                 gen_expr(it->expr)); 
                    } else{
                         assert(it->kind == FIELD_NAME);
                         result = bprintf("%s%s.%s=%s",result,\
                                  (it == expr->compound_expr.fields)?"":",",\
                                 gen_expr(it->field_expr),\
                                 gen_expr(it->expr)); 
                    }
               }
               result = bprintf("%s}",result);
               return result;
          }
          case EXPR_SIZEOF_TYPE:
               return bprintf("sizeof(%s)",type_to_cdecl(expr->sizeof_type.resolved_type,""));
               break;
          case EXPR_SIZEOF_EXPR:
               return bprintf("sizeof(%s)",gen_expr(expr->sizeof_expr));
               break;
     }
     return NULL;

}

#define paren_str(s,x) ( ( *s )?"("#x")":#x)
char *type_to_cdecl( Type *type , char *str ){
     
     switch ( type->kind ){
          case TYPE_INT:
               return bprintf("int%s%s",str?" ":"",str); 
          case TYPE_FLOAT:
               return bprintf("float%s%s\n",str?" ":"",str); 
          case TYPE_CHAR:
               return bprintf("char%s%s",str?" ":"",str); 
          case TYPE_VOID:
               return bprintf("void%s%s",str?" ":"",str); 
          case TYPE_PTR:
               return type_to_cdecl(type->ptr.base_type, bprintf(paren_str(str,*%s),str) );
          case TYPE_ARRAY:
               return type_to_cdecl(type->array.base_type, bprintf(paren_str(str,%s[%d]),str,type->array.size));
          case TYPE_FUNC:{
               char *result = NULL;
               result = bprintf("(*%s)(",str);
               if ( type->func.num_params == 0 ){
                    result = bprintf("%svoid",result);
               } else {
                    for (size_t i = 0; i < type->func.num_params; i++ ){
                    result = bprintf("%s%s%s",result,i==0?" ":",",type_to_cdecl(type->func.param_list[i],""));
                    }
               }
               result = bprintf("%s)",type_to_cdecl(type->func.ret_type,result));
               return result;
          }
          case TYPE_ENUM:
          case TYPE_STRUCT:
          case TYPE_UNION:
               return bprintf("%s %s",type->sym->name,str);
               break;
     }
}
#undef paren_str
bool need_semi_colon = true;
char *gen_code_var(Sym *sym){
     char *result = NULL;
     assert( sym->type );
     result = type_to_cdecl(sym->type,(char *)sym->name);
     if ( sym->decl->var_decl.expr ){
          result = bprintf("%s = %s",result,gen_expr(sym->decl->var_decl.expr));
     }
     result = bprintf("%s%s",result,need_semi_colon?";":"");
     return result;
}

char *gen_code_type(Sym *sym){
     assert(sym->kind == SYM_TYPE );
     Type *type = sym->type;
     Decl *decl = sym->decl;
     if ( decl->kind == DECL_STRUCT || decl->kind == DECL_UNION ){
          assert( type->kind == TYPE_STRUCT || type->kind == TYPE_UNION );
          char *result = NULL;
          result = bprintf("%s %s {\n",type->kind == TYPE_UNION?"union":"struct",sym->name); 
          for ( size_t i = 0 ; i < type->aggregate.num_fields; i++ ){
               TypeField it = type->aggregate.fields[i];
               result = bprintf("%s\t%s;\n",result,type_to_cdecl(it.type,(char *)it.name));
          }
          result = bprintf("%s};",result);
          return result;
     } else if ( decl->kind == DECL_TYPEDEF){
          return bprintf("typedef %s %s",type_to_cdecl(type,""),decl->name); 
     }
     return NULL;
}



char *gen_func_decl(Sym *sym){
     Decl *decl = sym->decl;
     Type *type = sym->type;
     char *result = NULL;
     result = bprintf("%s(",sym->name);
     assert( decl->func_decl.num_params == type->func.num_params );
     for ( size_t i = 0 ; i < type->func.num_params; i++ ){
          result = bprintf("%s%s%s",result,\
                    i == 0?"":",",\
                    type_to_cdecl( type->func.param_list[i], (char *)decl->func_decl.param_list[i].name ));
     }
     result = bprintf("%s)",result);
     return type_to_cdecl(type->func.ret_type,result);
}

#define semi_colon(x) ( (x)?";":"" )
void gen_stmt(Stmt *stmt){
     extern char *gen_stmt_block(StmtBlock);
     switch ( stmt->kind ){
          case STMT_INIT:{
               Sym *sym = stmt->init_stmt.sym;
               char *result = NULL;
               assert( sym->type );
               result = type_to_cdecl(sym->type,(char *)sym->name);
               assert( stmt->init_stmt.rhs);
               printf("%s = %s%s",result,gen_expr(stmt->init_stmt.rhs),semi_colon(need_semi_colon));
               break;
          }
          case STMT_EXPR:{
               printf("%s%s",gen_expr(stmt->expr_stmt),semi_colon(need_semi_colon));
               break;
          }
          case STMT_FOR:{
               printf("for (");
               if ( stmt->for_stmt.stmt_init ){
                    gen_stmt(stmt->for_stmt.stmt_init );
               } 
               if ( stmt->for_stmt.expr_cond ){
                    printf("%s;",gen_expr(stmt->for_stmt.expr_cond));
               }
               need_semi_colon = false; 
               if ( stmt->for_stmt.stmt_update ){
                    gen_stmt(stmt->for_stmt.stmt_update);
               }
               need_semi_colon = true;
               printf(")");
               gen_stmt_block(stmt->for_stmt.block);
               break;
          }
          case STMT_WHILE:
               printf("while ( %s )",gen_expr(stmt->while_stmt.expr_cond));
               gen_stmt_block(stmt->while_stmt.block);
               break;
          case STMT_DO_WHILE:
               printf("do ");
               gen_stmt_block(stmt->while_stmt.block);
               printf("(%s);",gen_expr(stmt->while_stmt.expr_cond));
               break;
          case STMT_IF:{
               printf("if ( %s )",gen_expr(stmt->if_stmt.cond));
               gen_stmt_block(stmt->if_stmt.if_block);
               for ( Elseif *it = stmt->if_stmt.elseifs;\
                         it != stmt->if_stmt.elseifs + stmt->if_stmt.num_elseifs;\
                         it++){
                    printf(" else if ( %s ) ",gen_expr(it->cond) );
                    gen_stmt_block(it->elseif_block);
               }
               if ( stmt->if_stmt.else_block.num_stmts != 0 ){
                    printf(" else ");
                    gen_stmt_block(stmt->if_stmt.else_block);
               }
               break;
          }
          case STMT_SWITCH:{
               printf("switch ( %s ){",gen_expr(stmt->switch_stmt.expr));
               indent++;
               new_line;
               for ( size_t i = 0; i < stmt->switch_stmt.num_cases; i++ ){
                    Case it= stmt->switch_stmt.cases[i];
                    if ( it.isdefault ){
                         printf("default :");
                         gen_stmt_block(it.case_block);
                         new_line;
                    }else {
                         for ( Expr **it1 = it.expr_list;\
                                   it1 != it.expr_list + it.num_expr;\
                                   it1++){
                              printf("case %s:",gen_expr(*it1));
                         }
                         if ( it.case_block.num_stmts != 0 ){
                              gen_stmt_block(it.case_block);
                         }
                         new_line;
                         
                    }
               }
               indent--;
               new_line;
               printf("}");
               break;
          }
          case STMT_RETURN:
               printf("return %s;",gen_expr(stmt->return_stmt.return_expr));
               break;
          case STMT_BREAK:
               printf("break;");
               break;
          case STMT_CONTINUE:
               printf("continue;");
               break;
          case STMT_BLOCK:
               gen_stmt_block(stmt->block);
               break;
          case STMT_DECL:
               need_semi_colon = false;
               printf("%s;",gen_code_var(stmt->decl_stmt->sym));
               need_semi_colon = true;
               break;
     }
}
#undef semi_colon
char *gen_stmt_block(StmtBlock block){
     printf("{");
     indent++;
     new_line;
     char *result = NULL;
     for ( size_t i = 0; i < block.num_stmts; i++ ){
          gen_stmt(block.stmts[i]);
          if ( i != block.num_stmts - 1 )
               new_line;
     }
     indent--;
     new_line;
     printf("}");
     return  result;
}

void gen_code_func(Sym *sym){
     Decl *decl = sym->decl;
     char *result = NULL;
     printf("%s", gen_func_decl(sym));
     char *result1 = gen_stmt_block(decl->func_decl.block); 
     printf("\n");
}

void gen_code_const(Sym *sym){
     Decl *decl = sym->decl;
     assert( decl->kind == DECL_CONST );
     printf("enum {");
     indent++;
     new_line;
     printf("%s",decl->name );
     if ( decl->const_decl.expr ){
          printf(" = %ld",sym->val);
     }
     indent--;
     new_line;
     printf("};");
     new_line;
}
char *gen_code(Sym *sym){
     switch ( sym->kind ){
          case SYM_VAR:{
               return gen_code_var(sym);
               break;
          }
          case SYM_TYPE:
               return gen_code_type(sym);
               break;
          case SYM_CONST:
               gen_code_const(sym); 
               break;
          case SYM_FUNC:
               gen_code_func(sym);
               break;
     }
     return NULL;
}

void type_gen_test(void){
     char *c1 = type_to_cdecl( type_int, "x");
     char *c2 = type_to_cdecl( type_ptr(type_int), "y");
     char *c3 = type_to_cdecl( type_ptr(type_ptr(type_int)), "y");
}

DeclList *parse_decls(void){
     Decl **tmp_decl_list = NULL;
     while ( !is_token(TOKEN_EOF) ){
          buff_push(tmp_decl_list,parse_decl());
     }
     size_t len = buff_len(tmp_decl_list);
     Decl **decl_list = arena_dup(&gen_arena, tmp_decl_list, sizeof(tmp_decl_list)*len);
     buff_free(tmp_decl_list);
     DeclList *tmp = arena_alloc(&gen_arena,sizeof(DeclList) );
     tmp->num_decls = len;
     tmp->decl_list = decl_list;
     return tmp;
}
void gen_test(void){
     sym_add_type(str_intern("int"),type_int);
     sym_add_type(str_intern("float"),type_float);
     sym_add_type(str_intern( "char" ),type_char);
     sym_add_type(str_intern( "void" ),type_void);
     type_gen_test();
     use_gen_buff = true;
     indent = 0;
     FILE *fp = fopen("out.c","w");
     char *list[] = {
//          "func foo(x:int,y:int):Vector{return {x,y};};",
//          "func foo1(x:int,y:int):Vector*{return &x;};",
//          "struct someArray { x:int[10];y:int;};",
//          "enum enumeration { X,Y,Z };",
//          "union IntorPtr { x :int; y : int *;};",
//          "var x : int = d",
//          "var y : int *",
//          "var z : int[10]",
//          "var a : (func (char*, char [10]):char);",
//          "var b : (func ():void)[10];",
//          "var c : Vector[10] ;",
//          "var d : int;",
//          "struct Vector {x:int;y:int;};",
//          "var e : newInt = 2",
//          "typedef newInt = int;",
//          "var f :int = 1+2*(3+2)",
//          "var g : int  = cast(int)2;",
//          "var h : Vector = foo(1,2);" ,
//          "var i =  &z[2];",
//          "var j = c[2].x;",
//          "var k = foo(1,2).x;",
//          "var l = (e == 2 )?3:4;",
//          "var m : Vector = { y = 1, x = 2 };",
//          "var n : int[10] = { [2] = 3, [3] = 4, [ 0] = 1};",
//          "var o : int[10] = { [3] = 4, [ 0] = 1};",
//          "var p : int = sizeof(:Vector)",
//          "var q  = sizeof(1+2);",
//          "var r : someArray [10];" ,
//          "var s : int = r[2].x[2];",
//          "func foo2(x:int,y:int):{ i := 2; i = 3;};"
/*          "func foo3(x:int,y:int){ i:=2; while ( i < 2 ){ i = i + 1; }}" ,
          "func foo4(x:int,y:int*){ i:=2; if ( i == 2 ){ while ( i < 2 ){i = i + 1;}} else if ( i == 3 ){i = 2;} else {i = 3;} }" ,
     
          "func foo5(x:int){ switch( x ){ case 1,2,3: x= x+1;break; default: x = 2;}}",*/
 //         "const x = 2;",
 //         "func factorial(x:int):int{ if  (x == 1){return 1;}else{ return x * factorial(x-1);} }",
//          "func main(x:int):int{ var e:int = 6; var y = factorial(e); return y; }",
  
  
#if 1 
      "union IntOrPtr { i: int; p: int*; };"
        "func f() {\n"
        "    u1 := IntOrPtr{i = 42};\n"
        "    u2 := IntOrPtr{p = cast(int*)u1.i};\n"
//        "    u1.i = 0;\n"
        //"var x : int = sizeof(:example);"
       //   "struct example { t : int; };"
//        "    u2.p = cast(int*)0;\n"
        "}\n"
        "var i: int;\n"
        "struct Vector { x: int; y: int; };\n"
#endif
//        "func fact_rec(n: int): int { r := 1; for (i := 2; i <= n; i++) { r *= i; }r++; return r; }\n",
 //       "func example_test(): int { return fact_rec(10); }\n",
#if 0
        "func f1() { v := Vector{1, 2}; j := i; i++; j++; v.x = 2*j; }\n",
        "func f2(n: int): int { return 2*n; }\n",
        "func f3(x: int): int { if (x) { return -x; } else if (x % 2 == 0) { return 42; } else { return -1; } }\n",
        "func f4(n: int): int { for (i := 0; i < n; i++) { if (i % 3 == 0) { return n; } } return 0; }\n",
        "func f5(x: int): int { switch(x) { case 0: case 1: return 42; case 3: default: return -1; } }\n",
        "func f6(n: int): int { p := 1; while (n) { p *= 2; n--; } return p; }\n",
        "func f7(n: int): int { p := 1; do { p *= 2; n--; } while (n); return p; }\n",
#endif
        "const z = 1+sizeof(p);\n"
        "var p: T;\n"
        "struct T { a: int[3]; };\n"
     };
     Decl *tmp_decl;
     init_stream(*list);
     DeclList *decls = parse_decls();
     for ( size_t i = 0 ; i < decls->num_decls ; i++ ){
          sym_add_decl( decls->decl_list[i]);
     }
     /*for( char **it = list; it != list + sizeof(list)/sizeof(char *); it++ ){
          init_stream(*it);
          tmp_decl = parse_decl();
          sym_add_decl(tmp_decl);
     }*/
    for ( Sym **it = global_sym_list; it != buff_end(global_sym_list); it++ ){
          resolve_sym(*it);
          if ( (*it)->kind == SYM_FUNC ){
               resolve_func(*it);
          }
          complete_type((*it)->type);
     }
     char *tmp = NULL;
    for( Sym **it = ordered_syms; it != buff_end(ordered_syms); it++ ){
//          print_decl( (*it)->decl );
          tmp = gen_code(*it);
          if ( tmp )
               printf("%s\n",tmp);          
    }
     flush_buff(fp,gen_buff);
}
