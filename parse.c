// 
// The parse function consumes the tokens they are supposed to parse
// For e.g parse_name on a:int will now consume a and current token will be :int after the function call
//
StmtBlock parse_stmt_block();
StmtBlock parse_stmt_list();
Stmt *parse_stmt();
Stmt *parse_stmt_if();
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
          //parse expr here
          Expr *expr = expr_int(token.int_val);
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
     expect_token('=');
     TypeSpec *type;
     type = parse_type();
     return decl_typedef(name,type);
     
}



Decl *parse_const(){
     const char *name = parse_name();
//     next_token();
     expect_token('=');
     Expr *expr = NULL;
          if ( is_token(TOKEN_INT) ) // parse expr here
               expr = expr_int(token.int_val);
          else if ( is_token(TOKEN_NAME) )
               expr = expr_name(token.name);     
     return decl_const(name,expr);
}

Decl *parse_var(){
     const char *name = parse_name();
     TypeSpec *type = NULL;
     Expr *expr = NULL;
     if ( match_token(':') ){
         type = parse_type();// 
     }
     if ( match_token('=') ){
          if ( is_token(TOKEN_INT) ) // parse expr here
               expr = expr_int(token.int_val);
          else if ( is_token(TOKEN_NAME) )
               expr = expr_name(token.name);     
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
          if ( match_token('=') ){
               if ( is_token(TOKEN_INT) ){ // to be replaced by actual parse_expr function
                   expr = expr_int(token.int_val);
               } else {
                    syntax_error("Not valid Expression assignment for enum item");
               }
               next_token();
          }
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
Expr *parse_expr(){
     next_token();
     return expr_int(1);
}
Case parse_switch_case(){
     Case new_case = {0};
     static bool is_default = false;
     StmtBlock block;
     Stmt **stmt_list = NULL;
     Expr **expr_list = NULL;
     while ( match_keyword("case") ){ // allows for case 1: case 2: ...
           buff_push(expr_list,parse_expr());
           while ( match_token(',') ){
                buff_push(expr_list,parse_expr());
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
     Expr *expr = expr_name("a");
     next_token();
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
     Expr *lhs = expr_name("abc");
     next_token();
     if ( match_token(TOKEN_COLON_ASSIGN) ){
          if ( lhs->kind != EXPR_NAME ){
               syntax_error(":= must be preceded by a name");
          }
          //parse expr
          next_token();
          return stmt_init(lhs->name,expr_int(1));
     } else if ( is_token(TOKEN_INC) || is_token(TOKEN_DEC) ){
          TokenKind op = token.kind;
          next_token();
          return stmt_assign(op,lhs,NULL);
     } else if ( is_assign_op() ){
          TokenKind op = token.kind;
          next_token();
          Expr *rhs= parse_expr();
          return stmt_assign(op,lhs,rhs);
     } else {
          return NULL;
     }
}
Stmt *parse_stmt_for(){
     expect_token('(');
     Stmt *init = NULL;
          init = parse_stmt_simple();
          expect_token(';');
     Expr *cond = NULL;
     if ( !is_token(';') ){
        cond = expr_binary(TOKEN_EQ,expr_name("a"),expr_int(23)); 
     }
     next_token();// remove
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
     
      Expr *test_expr = expr_binary(EQ,expr_name("a"),expr_int(12));
     Expr *cond = test_expr;
     expect_token(')');
     StmtBlock block = parse_stmt_block();
     return stmt_while(cond,block);
}

Stmt *parse_stmt_do_while(){
     StmtBlock block  = parse_stmt_block();
     expect_keyword("while");
     expect_token('(');
     // parse expr
     
     Expr *test_expr = expr_binary(EQ,expr_name("a"),expr_int(12));
     Expr *cond = test_expr;
     expect_token(')'); 
     expect_token(';');
     return stmt_do_while(cond,block);
}

Stmt *parse_stmt_if(){
     expect_token('(');
     // parse expr
      Expr *test_expr = expr_binary(EQ,expr_name("a"),expr_int(12));
     Expr *expr = test_expr;
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
               expr = test_expr;
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
     Expr *expr = NULL;
     if ( is_token(TOKEN_NAME) ){
          expr = expr_name(parse_name());
     } else if ( is_token(TOKEN_INT) ){
          expr = expr_int(token.int_val);
     }
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
          //"var x:(func (char,char):int) = 12",
          //"enum abc { FUCK, THIS = 23, SHIT, YOU = 12, FUCKING= 14, ASS = 9}",
          //"struct abc { x,y:int; z:char;} ",
          //"union abc { x,y:int; z:char;} ",
          //"var x:int = 1234",
          //"var x:Vector",
      //"var x:char = abc",
      //"const x = 1234",
      //"typedef cell = int32",
//        "func abc ( a:int,b:char) : int {}",
//        "func xyz ( a:int,b:char) {}",
//        "func xyz ( ) {}",
//        "func xyz ( ):char {if(){break;continue;}else if (){break;}else{continue;}}",
//        "func xyz ( ):char {if(){break;continue;}else{break;}}",
//        "func xyz ( ):char {if(){break;continue;}else if(){break;}}",
//        "func foo(a:int,b:int):char{while(){if(){continue;}}}",
//        "func do_while(a:char,b:char):int{do { continue; }while();}",
//        "func foo (a:char*,b:size_t):char*{break;continue;}",
//          "func foo (a:char,b:char):int{for(i:=0;x;i++){continue;}}", 
            "func foo (a:char,b:char):int{for(i:=0;x;i++){switch ( a ){ case a,b,c:x+=1 ;break; case b:case c: break; default:break;}}" 
     };
     Decl *temp_decl; 
     for ( char **it =  parse_string; it!=parse_string+sizeof(parse_string)/sizeof(char *); it++ ){
          init_stream(*it);
          temp_decl = parse_decl();
          print_decl(temp_decl);
          putchar('\n');
     }

}
