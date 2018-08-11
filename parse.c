// 
// The parse function consumes the tokens they are supposed to parse
// For e.g parse_name on a:int will now consume a and current token will be :int after the function call
//
StmtBlock parse_stmt_block();
StmtBlock parse_stmt_list();
TypeSpec *parse_type();
Stmt *parse_stmt();
Stmt *parse_stmt_if();
Expr *parse_expr();
Expr *parse_expr_test(){
     return parse_expr();
}


Expr *parse_expr_operand(){
     Expr *operand;
      if  ( is_token(TOKEN_INT) ){
               operand = expr_int(token.int_val); 
               next_token();
               return operand;
      }else if ( is_token( TOKEN_FLOAT ) ){
               operand = expr_float(token.float_val);
               next_token();
               return operand;
      } else if ( is_token(TOKEN_STR) ){
               operand = expr_str(token.str_val);
               next_token();
               return operand;
      } else if ( is_token( TOKEN_NAME ) ){
               const char *name = token.name;
               next_token();
               if ( match_token('{') ){
                    Expr **expr_list = NULL;
                    buff_push(expr_list,parse_expr());
                    while ( !is_token('}') && !is_token(TOKEN_EOF) ){
                         match_token(',');
                         buff_push(expr_list,parse_expr());
                    }
                    expect_token('}');
                    return expr_compound( type_name(name),expr_list,buff_len(expr_list));
               } else {
                    return expr_name(name);
               }
      } else if ( match_token('(') ){ 
               Expr *new_expr = NULL;
               if ( match_token(':') ){
                    TypeSpec *type = parse_type();
                    expect_token(')');
                    new_expr = parse_expr();
                    return expr_cast(type,new_expr);
               }
               new_expr = parse_expr();
               expect_token(')');
               return new_expr;
          } else if ( match_token('{') ){
                     Expr **list = NULL;
                     buff_push(list,parse_expr());
                     while ( !is_token(TOKEN_EOF)  && !is_token('}') ){
                          match_token(',');
                          buff_push(list,parse_expr());
                          
                     }
                     expect_token('}');
                   return expr_compound(NULL,list,buff_len(list));
          } else if ( match_keyword("sizeof") ){
               expect_token('(');
               if ( match_token(':')){
                   TypeSpec *type = parse_type();
                  expect_token(')');
                 return expr_sizeof_type(type); 
               } else {
                    Expr *expr = parse_expr();
                    expect_token(')');
                    return expr_sizeof_expr(expr);
               }
          }
      syntax_error("Expected Expression but didn't get any thing");
      return NULL;
}

Expr **parse_expr_list(){
     Expr **list = NULL;
     buff_push(list,parse_expr());
     while ( !is_token(TOKEN_EOF) && match_token(',') ){
          buff_push(list,parse_expr());
     }
     return list;
}

Expr *parse_expr_base(){
     Expr *operand = parse_expr_operand();
     if ( match_token('(') ){
          Expr **expr_list = NULL;
          buff_push(expr_list,parse_expr());
          while ( !is_token(')') && !is_token(TOKEN_EOF) ){
               match_token(',');
               buff_push(expr_list,parse_expr());
          }
     //     Expr **expr_list = parse_expr_list(); 
          expect_token(')');
          operand = expr_call(operand,expr_list,buff_len(expr_list));
     } else if ( match_token('[') ){
          operand = expr_index(operand,parse_expr());
          expect_token(']');
     } else if ( match_token('.') ){
          if ( is_token(TOKEN_STR) ){
               operand = expr_field(operand,token.str_val);
          } else {
               syntax_error("Field access requires name to be final token\n");
          }
          next_token();
     } 

     return operand;
}

Expr *parse_expr_unary(){
     Expr *unary_expr;
     if ( is_unary_op() ){
          TokenKind op = token.kind;
          next_token();
          unary_expr = expr_unary(op,parse_expr_unary());
     } else {
          unary_expr = parse_expr_base();
     }
     return unary_expr;
}

Expr *parse_expr_mul(){
     Expr *mul_expr = parse_expr_unary();
     while ( is_mul_op() ){
          TokenKind op = token.kind;
          next_token();
          mul_expr = expr_binary(op,mul_expr,parse_expr_unary());
     }
     return mul_expr;
}

Expr *parse_expr_add(){
     Expr *add_expr = parse_expr_mul();
     while ( is_add_op() ){
          TokenKind op = token.kind;
          next_token();
          add_expr = expr_binary(op,add_expr,parse_expr_mul());
     }
     return add_expr;
}

Expr *parse_expr_comp(){
     Expr *comp_expr = parse_expr_add();
     while ( is_comp_op() ){
          TokenKind op = token.kind;
          next_token();
          comp_expr = expr_binary(op,comp_expr,parse_expr_add());
     }
     return comp_expr;
}
Expr *parse_expr_and(){
     Expr *and_expr = parse_expr_comp();
     while ( match_token(TOKEN_AND) ){
          //TokenKind op = token.kind;
          and_expr = expr_binary(TOKEN_AND,and_expr,parse_expr_comp());
     }
     return and_expr;
}

Expr *parse_expr_or(){
     Expr *or_expr = parse_expr_and();
     while ( match_token(TOKEN_OR) ){
//          TokenKind op = token.kind;
          or_expr = expr_binary(TOKEN_OR,or_expr,parse_expr_and());
     }
     return or_expr;
}

Expr *parse_expr_ternary(){
     Expr *ternary_expr = parse_expr_or();
     if ( match_token('?') ){
          Expr *then_expr = parse_expr_ternary();
          expect_token(':');
          Expr *else_expr = parse_expr_ternary();
          return expr_ternary(ternary_expr,then_expr,else_expr);
     } else {
          return ternary_expr;
     }
}

Expr *parse_expr(){
     return parse_expr_ternary();
}

const char *parse_name(){
     if ( is_token(TOKEN_NAME) ){
          const char *name = token.name;
          next_token();
          return token.name;
     }
     else
          syntax_error("Expected TOKEN_NAME, got crap instead");
     return NULL;
}
TypeSpec *parse_type();
TypeSpec *parse_base_type(){
     if ( is_token(TOKEN_NAME) ){
          const char *name = token.name;
          next_token();
         return type_name(name) ;
     } else if ( match_keyword("func") ){
          TypeSpec **type_list = NULL;
          expect_token('(');
          buff_push(type_list,parse_type());
          while ( !is_token(')') && !is_token(TOKEN_EOF) ){
               expect_token(',');
               buff_push(type_list,parse_type());
          }
          expect_token(')');
          expect_token(':');
          TypeSpec *ret_type = parse_type();
          return type_func(type_list,buff_len(type_list),ret_type);
     } else {
          expect_token('(');
          TypeSpec *type = parse_base_type();
          expect_token(')');
          return type;
     }
}

TypeSpec *parse_type(){
     TypeSpec *type = parse_base_type();
     if ( match_token('*') ){
          type = type_pointer(type);
     } else if ( match_token('[') ){
          Expr *expr = parse_expr();
          type = type_array(type,expr);
          expect_token(']');
     }
     return type;
}

func_param parse_func_param(){
     const char *name = parse_name();
     expect_token(':');
     TypeSpec *type = parse_type(); 
     return (func_param){name,type};
}

Decl *parse_func_decl(){
     const char *name = parse_name();
     expect_token('(');
     func_param *list = NULL;
     if ( !is_token(')') ){
          buff_push(list,parse_func_param());
          while ( !is_token(')') && !is_token(TOKEN_EOF) ){
               expect_token(',');
               buff_push(list,parse_func_param());
          }
     }
     expect_token(')');
     TypeSpec *ret_type = NULL;
     if ( match_token(':') ){
          ret_type = parse_type();// 
     }
     StmtBlock block = parse_stmt_block(); // parse block
     return  decl_func(name,ret_type,list,buff_len(list),block);
}
Decl *parse_typedef(){
     const char *name = parse_name();
     expect_token(TOKEN_ASSIGN);
     TypeSpec *type;
     type = parse_type();
     return decl_typedef(name,type);
     
}



Decl *parse_const(){
     const char *name = parse_name();
//     next_token();
     expect_token(TOKEN_ASSIGN);
     Expr *expr = parse_expr();
     
     return decl_const(name,expr);
}

Decl *parse_var(){
     const char *name = parse_name();
     TypeSpec *type = NULL;
     Expr *expr = NULL;
     if ( match_token(':') ){
         type = parse_type();// 
     }
     if ( match_token(TOKEN_ASSIGN) ){
          expr = parse_expr();
     }
     return decl_var(name,type,expr);     
}


aggregate_item parse_aggregate_item(){
     const char **name = NULL;
     buff_push(name,parse_name());
     while( match_token(',') ){
          buff_push(name,parse_name());
     }
     expect_token(':');
     TypeSpec *type = parse_type();
     
     return (aggregate_item){name,buff_len(name),type};
}

Decl *parse_agg(StmtKind kind){
     Decl *new_agg;
     const char *name = parse_name();
     aggregate_item *list = NULL;
     expect_token('{');
     while ( !match_token('}') && !is_token(TOKEN_EOF) ){
          buff_push(list,parse_aggregate_item());
          expect_token(';');
     }
     new_agg = decl_aggregate(kind,name,buff_len(list),list);
     return new_agg;
}

enum_item parse_enum_item(){
     const char *name;
     Expr *expr = NULL ;
     if ( is_token(TOKEN_NAME) ){
          name = token.name;
          next_token();
          if ( match_token(TOKEN_ASSIGN) ){
                   expr = parse_expr();
          }
         //      next_token();
          
     } else {
          syntax_error("Not valid name for enum");
     }
     return new_enum(name,expr) ;
     
}
Decl *parse_enum(){
     Decl *new_enum = decl_new(DECL_ENUM,token.name);
     next_token(); 
     expect_token('{');
     enum_item *enum_item_list = NULL;
     while ( !match_token('}') ){
          buff_push(enum_item_list,parse_enum_item());
          match_token(',');
     }
     new_enum->enum_decl.enum_items = enum_item_list;
     new_enum->enum_decl.num_enum_items = buff_len(enum_item_list);
    return new_enum; 
}


Decl *parse_decl(){
     Decl *new_decl = NULL;
     if ( match_keyword("enum") ){
          new_decl = parse_enum();
     } else if ( match_keyword("struct") ){
          new_decl = parse_agg(DECL_STRUCT); 
     } else if ( match_keyword("union") ){
          new_decl = parse_agg(DECL_UNION);
     } else if ( match_keyword("var") ){
          new_decl = parse_var();
     } else if ( match_keyword("const")) {
          new_decl = parse_const();
     } else if ( match_keyword("typedef")){
          new_decl = parse_typedef();
     } else if ( match_keyword("func") ){
          new_decl = parse_func_decl();
     }
    return new_decl; 

}


StmtBlock parse_stmt_block(){
     
     //Stmt **stmt1 = stmt_list(3,stmt_break(),stmt_expr(expr_name("i")),stmt_while(expr_binary(EQ,expr_name("ab"),expr_int(1)),new_block(1,stmt_list(1,stmt_continue()))));
     //
     //StmtBlock block_test1 = new_block(3,stmt1 );
     StmtBlock block = {0}; 
     expect_token('{');
     while ( !is_token('}') && !is_token(TOKEN_EOF) ){
          buff_push(block.stmts,parse_stmt());
     }
     block.num_stmts = buff_len(block.stmts);
     expect_token('}');
     return block;
}
Case parse_switch_case(){
     Case new_case = {0};
     bool is_default = false;
     StmtBlock block;
     Stmt **stmt_list = NULL;
     Expr **expr_list = NULL;
     while ( match_keyword("case") ){ // allows for case 1: case 2: ...
           buff_push(expr_list,parse_expr_test());
           while ( match_token(',') ){
                buff_push(expr_list,parse_expr_test());
           }
           expect_token(':'); 
     }
     new_case.expr_list = expr_list;
     new_case.num_expr = buff_len(expr_list);
     if ( match_keyword("default") ){
          if ( is_default ){
               syntax_error("Cant have duplicate definitions of default in same case\n");
          }
          is_default = true;
          new_case.isdefault = true;
          expect_token(':');
     }
     while ( !is_token(TOKEN_EOF) && !is_token('}') && !is_keyword("case") && !is_keyword("default") ){
          buff_push(stmt_list,parse_stmt());
     }
     block.stmts = stmt_list;
     block.num_stmts = buff_len(stmt_list);
     new_case.case_block = block;
     return new_case;
     
}
Stmt *parse_stmt_switch(){
     expect_token('(');
     //parse expr
     Expr *expr = parse_expr();
     expect_token(')');
     expect_token('{');
     Case *case_list = NULL;
     while ( !is_token('}') && !is_token(TOKEN_EOF) ){
          buff_push(case_list,parse_switch_case());
     }
     expect_token('}');
     return stmt_switch(expr,buff_len(case_list),case_list);
}

Stmt *parse_stmt_simple(){
     //parse expr
     Expr *lhs = parse_expr();
     if ( match_token(TOKEN_COLON_ASSIGN) ){
          if ( lhs->kind != EXPR_NAME ){
               syntax_error(":= must be preceded by a name");
          }
          //parse expr
         // next_token();
          return stmt_init(lhs->name,parse_expr());
     } else if ( is_token(TOKEN_INC) || is_token(TOKEN_DEC) ){
          TokenKind op = token.kind;
          next_token();
          return stmt_assign(op,lhs,NULL);
     } else if ( is_assign_op() ){
          TokenKind op = token.kind;
          next_token();
          Expr *rhs= parse_expr_test();
          return stmt_assign(op,lhs,rhs);
     } else {
          if ( lhs->kind == EXPR_CALL ){
               return stmt_expr(lhs);
          }
          syntax_error("Blank Statement has no use\n");
     }
}
Stmt *parse_stmt_for(){
     expect_token('(');
     Stmt *init = NULL;
     init = parse_stmt_simple();
     expect_token(';');
     Expr *cond = NULL;
     if ( !is_token(';') ){
          cond = parse_expr();
     }
    expect_token(';');
    Stmt *update = NULL;
    if ( !is_token(')') ){
         update = parse_stmt_simple();
         if ( update->kind == STMT_INIT){
              syntax_error("Initialization is not allowed in for loop\n");
         }
    }
    expect_token(')');
    StmtBlock block = parse_stmt_block();
    return stmt_for(init,cond,update,block);
}
Stmt *parse_stmt_while(){
     expect_token('(');
     //parse expr
     
     Expr *cond = parse_expr();
     expect_token(')');
     StmtBlock block = parse_stmt_block();
     return stmt_while(cond,block);
}

Stmt *parse_stmt_do_while(){
     StmtBlock block  = parse_stmt_block();
     expect_keyword("while");
     expect_token('(');
     // parse expr
     
     Expr *cond = parse_expr();
     expect_token(')'); 
     expect_token(';');
     return stmt_do_while(cond,block);
}

Stmt *parse_stmt_if(){
     expect_token('(');
     // parse expr
     Expr *expr = parse_expr();
     expect_token(')');
     StmtBlock if_block = parse_stmt_block();
     Elseif *elseifs = NULL;
     Elseif temp;
     StmtBlock block_elseif; 
     StmtBlock else_block={};
     while ( match_keyword("else") ){
          if ( match_keyword("if") ){
               expect_token('(');
               // parse expr
               expr = parse_expr();
               expect_token(')');
               block_elseif = parse_stmt_block();
               temp = (Elseif){expr,block_elseif};
               buff_push(elseifs,temp); 
          } else {
               else_block = parse_stmt_block();
               break;
          }

     }
     return stmt_if(expr,if_block,buff_len(elseifs),elseifs,else_block);
}

Stmt *parse_return_stmt(){
     // parse expr
     Expr *expr = parse_expr();
     //if ( is_token(TOKEN_NAME) ){
     //     expr = expr_name(parse_name());
     //} else if ( is_token(TOKEN_INT) ){
     //     expr = expr_int(token.int_val);
     //}
     expect_token(';');
     return  stmt_return(expr);
}

Stmt *parse_stmt(){
     if ( match_keyword("return") ){
          return parse_return_stmt();
     } else if ( match_keyword("if") ){
          return parse_stmt_if();
     } else if ( match_keyword("while") ){
          return parse_stmt_while();
     } else if ( match_keyword("for") ){
          return parse_stmt_for();
     } else if ( match_keyword("do") ){
          return parse_stmt_do_while();
     } else if ( match_keyword("switch") ){
          return parse_stmt_switch();
     } else if ( match_keyword("break") ){
          expect_token(';');
          return stmt_break();
     } else if ( match_keyword("continue") ){
          expect_token(';');
          return stmt_continue();
     } else {
          // parse expr
          Stmt *new = parse_stmt_simple();
          expect_token(';'); 
          return new;
     }
}


void parse_test(){
     init_intern_keyword();
     assert(is_token_keyword("typedef"));
     assert( !is_token_keyword("fucnthisshit"));
     char *parse_string[] = {
          "var x:(func (char,char):int) = 12",
          "enum abc { FUCK, THIS = 23, SHIT, YOU = 12, FUCKING= 14, ASS = 9}",
          "struct abc { x,y:int; z:char;} ",
          "union abc { x,y:int; z:char;} ",
          "var x:int = 1234",
          "var x:Vector",
      "var x:char = abc",
      "const x = 1234",
     "typedef cell = int32",
        "func abc ( a:int,b:char) : int {}",
        "func xyz ( a:int,b:char) {}",
        "func xyz ( ) {}",
        "func xyz ( ):char {if(x!=2){break;continue;}else if (x!=2){break;}else{continue;}}",
        "func xyz ( ):char {if(x!=2){break;continue;}else{break;}}",
        "func xyz ( ):char {if(x!=2){break;continue;}else if(x!=2){break;}}",
        "func foo(a:int,b:int):char{while(a>=2){if(x!=2){continue;}}}",
        "func do_while(a:char,b:char):int{do { continue; }while(x!=2);}",
        "func foo (a:char*,b:size_t):char*{break;continue;}",
          "func foo (a:char,b:char):int{for(i:=0;x;i++){continue;}}", 
            "func foo (a:char,b:char):int{for(i:=0;x;i++){switch ( o ){ case a,b,c:x+=1 ;break; case b:case c: break; default:break;}}}" ,
         "func foo (a:int):int{ x:=x+2; }",
         "func foo (a:int):int{ x = y==2 ? a : b(~x,*y); }"
         " func foo (a:int *):int{ x <<= *x+2*3 ;}"
         " func fact(n:int):int { trace(\"fact\"); if ( n== 0 ){return 1;} else { return n*fact(n-1);}}",
         "func fact(n:int):int { p := 1; for(i:=1; i<=n; i++){p*=1;} return p;}",
          " var foo = a?a&b + c<<d + e*f == +u-v-w + *g/h(x,y) + -i%k[x] && m<=n*(p+q/r) : 0",
          " func f(x:int):bool{switch(x){case 0:case 1: return true; case 2: default: return false;}}",
         "enum Color { RED = 3, GREEN, BLUE = 0}",
         " var x = b==1?1+2:3-4",
         "const pi = 3.14",
         "struct Vector {x,y:float;}",
          "var v : Vector = {1.0,-1.0}",
          "union IntOrFloat { i:int; f:float;}",
          "typedef Vectors = Vector[1+2]",
          "var v:int = **x",
          "func fact(x:int):int{while(x!=1){product*=x;x = x - 1 ;}}",
          "const x = sizeof(:int)",
          "var x:int = sizeof(3)",
          //"var x = ",
          "func fact(x:int):int{2;}"
               
     };
     Decl *temp_decl; 
     for ( char **it =  parse_string; it!=parse_string+sizeof(parse_string)/sizeof(char *); it++ ){
          init_stream(*it);
          temp_decl = parse_decl();
          print_decl(temp_decl);
          putchar('\n');
     }

}
