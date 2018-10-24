#define INT_SIZE 4
#define FLOAT_SIZE 4
#define CHAR_SIZE 1
#define VOID_SIZE 0
#define PTR_SIZE 8
#define INT_ALIGNMENT 4
#define FLOAT_ALIGNMENT 4
#define PTR_ALIGNMENT 8

#include "list.c"
Arena resolve_arena;
typedef enum TypeKind {
     TYPE_NONE,
     TYPE_INCOMPLETE,
     TYPE_COMPLETING,
     TYPE_ENUM,
     TYPE_INT,
     TYPE_FLOAT,
     TYPE_CHAR,
     TYPE_VOID,
     TYPE_PTR,
     TYPE_ARRAY,
     TYPE_FUNC,
     TYPE_CONST,
     TYPE_VAR,
     TYPE_STRUCT,
     TYPE_UNION
} TypeKind;

typedef struct {
     Type *type;
     const char *name;
} TypeField;

struct Type {
     TypeKind kind;
     size_t size;
     size_t alignment;
     Sym *sym;
     union {
          struct {
               Type *base_type;
          }ptr;
          struct {
               Type *base_type;
               size_t size;
          } array;
          struct {
               TypeField *fields;
               size_t num_fields;
          }aggregate;
          struct {
               Type **param_list;
               Type *ret_type;
               size_t num_params;
          } func;
     };
};

typedef enum SymKind {
     SYM_NONE,
     SYM_CONST,
     SYM_VAR,
     SYM_FUNC,
     SYM_TYPE,
     SYM_ENUM_CONST
} SymKind;

typedef enum SymState {
     SYM_UNRESOLVED,
     SYM_RESOLVING,
     SYM_RESOLVED
} SymState;


struct Sym {
     const char *name;
     SymKind kind;
     SymState state;
     Decl *decl;
     Type *type;
     int64_t val;
} ;


typedef struct ResolvedExpr {
     Type *type;
     bool is_lvalue;
     bool is_const;
     union {
          int64_t int_val;
          double float_val;
     };
} ResolvedExpr;


Type *type_alloc(TypeKind kind){
     Type *new = arena_alloc(&resolve_arena,sizeof(Type));
     new->kind = kind;
     return new;
}


typedef struct CachePtrType {
     Type *ptr;
     Type *base_type;
}CachePtrType;
CachePtrType *ptr_list = NULL;

Type *type_ptr(Type *base_type){
     for (CachePtrType *it = ptr_list; it!=buff_end(ptr_list) ; it++ ){
          if  ( it->base_type == base_type){
               return it->ptr;
          }
     }

     Type *new = type_alloc(TYPE_PTR);
     new->ptr.base_type = base_type;
     new->size = PTR_SIZE;
     new->alignment = PTR_ALIGNMENT;
     CachePtrType tmp = { new , base_type };
     buff_push(ptr_list,tmp);
     return new; 
}

typedef struct CacheFuncType {
     Type *func;
     Type **param_list;
     size_t num_params;
     Type *ret_type;
} CacheFuncType;

CacheFuncType *func_list = NULL;

Type *type_func(Type **param_list,size_t num_params, Type *ret_type){
     for ( CacheFuncType *it = func_list; it != buff_end(func_list); it++ ){
          if ( it->num_params != num_params || it->ret_type != ret_type ){
               break;
          }
          assert(num_params == it->num_params && it->ret_type == ret_type);
          bool match = true;
          for ( size_t i = 0; i < num_params; i++ ){
               if ( it->param_list[i] != param_list[i] ){
                    match = false;
                    break;
               }
          }
          if ( match ){
               return it->func;
          } 
     }

     Type *new = type_alloc(TYPE_FUNC);
     new->size = PTR_SIZE;
     new->alignment = PTR_ALIGNMENT;
     new->func.param_list = param_list;
     new->func.num_params = num_params;
     new->func.ret_type = ret_type;
     CacheFuncType tmp = {new,param_list,num_params,ret_type};
     buff_push(func_list,tmp);
     return new;
}


typedef struct CacheArrayType{
     Type *array;
     Type *base_type;
     size_t size; 
} CacheArrayType;

CacheArrayType *array_list = NULL;
Type *type_array(Type *base_type, size_t size ){
     for ( CacheArrayType *it = array_list; it != buff_end(array_list) ; it++ ){
          if ( it->size == size && it->base_type==base_type ){
               return it->array;
          }
     }

     Type *new = type_alloc(TYPE_ARRAY);
     new->array.base_type = base_type;
     new->size = size * base_type->size;
     new->alignment = base_type->alignment;
     new->array.size = size;
     CacheArrayType tmp = { new,base_type,size };
     buff_push(array_list,tmp);
     return new;
}

Type *type_incomplete(Sym *sym){
     Type *new = type_alloc(TYPE_INCOMPLETE);
     new->sym = sym;
     return new;
}

Type INT_TYPE = { TYPE_INT, INT_SIZE, .alignment = INT_ALIGNMENT};
Type FLOAT_TYPE = {TYPE_FLOAT, FLOAT_SIZE, .alignment = FLOAT_ALIGNMENT};
Type CHAR_TYPE = { TYPE_CHAR, CHAR_SIZE}; 
Type VOID_TYPE = { TYPE_VOID, VOID_SIZE};
Type ENUM_TYPE  = { TYPE_ENUM, INT_SIZE , .alignment = INT_ALIGNMENT };
Type *type_int = &INT_TYPE;
Type *type_float = &FLOAT_TYPE;
Type *type_char = &CHAR_TYPE;
Type *type_void = &VOID_TYPE;
Type *type_enum = &ENUM_TYPE;




static Sym **global_sym_list = NULL;
// Stack for variable scoping
enum { MAX_LOCAL_DEF = 1024 };
SymNode *local_sym_list[MAX_LOCAL_DEF];
SymNode **last_local_sym = local_sym_list;
bool is_local_scope = false;

SymNode **sym_enter( void ){
     /*
      * Executes when we enter a new scope
      */
     SymNode **tmp = last_local_sym; 
     if ( last_local_sym == local_sym_list + MAX_LOCAL_DEF ){
          fatal("Max. no of scopes variables reached!");
     }
     last_local_sym++;
     return tmp;
}
Sym *sym_add_local(Sym *sym){
     assert( last_local_sym - 1 >= local_sym_list );
     SymNode **current = ( last_local_sym - 1 );
     *current = addSymNode(*current, newSymNode(sym) );
     return sym;
}

SymNode **sym_exit(SymNode **sym){
     *(last_local_sym-1) = NULL;
     last_local_sym--;
     return sym;
}

bool in_local(const char *name){
     for ( SymNode *it = *(last_local_sym - 1); it != NULL ;it = it->next){
          if ( it->sym->name == name ){
               return true;
          }
     }
     return false;
}


Sym *sym_get(const char *);
Sym *new_sym(SymKind kind, const char *name, Decl *decl){
     Sym *new = arena_alloc(&resolve_arena,sizeof(Sym));
     new->kind = kind;
     new->state = SYM_UNRESOLVED;
     new->name = name;
     new->decl = decl;
     new->type = NULL;
     return new;
}
Sym *sym_var(const char *name, Type *type){
    Sym *new = arena_alloc(&resolve_arena,sizeof(Sym)); 
    new->kind = SYM_VAR;
    new->name = name;
    new->type = type;
    new->state = SYM_RESOLVED;
    return new;
}


Sym *sym_add_decl(Decl *decl){
     if ( sym_get(decl->name) ){
          resolve_error(&decl->location,"Duplicate var names, %s\n",decl->name);
     } 
     extern void sym_add_type(const char *,Type *);
     SymKind kind = SYM_NONE;
     switch ( decl->kind ){
          case DECL_ENUM:
          case DECL_TYPEDEF:
          case DECL_STRUCT:
          case DECL_UNION:
               kind = SYM_TYPE;
               break;
          case DECL_CONST:
               kind = SYM_CONST;
               break;
          case DECL_VAR:
               kind = SYM_VAR;
               break;
          case DECL_FUNC:
               kind = SYM_FUNC;
               break;
          default:
               assert(0);
               break;
     }
     Sym *new = new_sym(kind,decl->name,decl);
     if ( decl->kind == DECL_STRUCT || decl->kind == DECL_UNION ){
          /*
           * For struct and union we use an incomplete type to denote it.
           * The type gets completed if the declaration is complete.
           * e.g. 'struct type' is an incomplete type
           *
           */
          new->type = type_incomplete(new);
          new->state = SYM_RESOLVED;  
     }
     buff_push(global_sym_list,new);
     if  ( decl->kind == DECL_ENUM ){
          Sym *tmp;
          for ( size_t i = 0; i < decl->enum_decl.num_enum_items; i++ ){
               tmp = new_sym(SYM_ENUM_CONST,decl->enum_decl.enum_items[i].name,decl);
               tmp->state = SYM_RESOLVED;
               tmp->type = type_int;
               buff_push(global_sym_list,tmp); 
          
          }
     }
     return new;

}
Type *resolve_typespec(TypeSpec *);
Sym *sym_get(const char *name){
     for ( SymNode **it = last_local_sym; it >= local_sym_list; it-- ){
          SymNode *current = *(it - 1);
          for ( SymNode *it1 = current; it1 != NULL ; it1 = it1->next){
               if ( it1->sym->name == name ){
                    return it1->sym;
               }
          }
     }
     for ( Sym **it = global_sym_list; it != buff_end(global_sym_list); it++ ){
          if ( (*it)->name == name ){
               return *it;
          }
     } 
     return NULL;
}


Sym *resolve_name(const char *name){
     Sym *new = sym_get(name);
     if ( !new ){
          fatal("%s is not defined!!\n",name);
     }
     return new;
}

static Sym **ordered_syms = NULL;
ResolvedExpr resolved_null = {};
ResolvedExpr resolve_expr(Expr *);
ResolvedExpr resolve_expr_expected(Expr *,Type *);

void duplicate_field_check(Type *type){
     assert(type->kind == TYPE_STRUCT || type->kind == TYPE_UNION );
     for ( TypeField *it = type->aggregate.fields;  it!= type->aggregate.fields + type->aggregate.num_fields; it++ ){
          for ( TypeField *it2 = it+1;  it2!= type->aggregate.fields + type->aggregate.num_fields; it2++ ){
               if ( it->name == it2->name ){
                    fatal("Duplicate field names in the structure %s!\n",type->sym->name );
               }
          }
     }
}
void complete_type_struct(Type *type,TypeField *fields, size_t num_fields){
     assert(type->kind == TYPE_COMPLETING);
     type->kind = TYPE_STRUCT;
     type->size = 0;
     for ( size_t i = 0; i < num_fields-1; i++ ){
          type->alignment = MAX(fields[i].type->alignment, fields[i+1].type->alignment);
     }
     for ( size_t i = 0; i < num_fields; i++ ){
          type->size += ALIGN_UP(fields[i].type->size,type->alignment);
     }
     type->aggregate.fields = arena_dup(&resolve_arena,fields,sizeof(*fields)*num_fields);
     type->aggregate.num_fields = num_fields; 
}

void complete_type_union(Type *type, TypeField *fields, size_t num_fields){
     assert(type->kind == TYPE_COMPLETING);
     type->kind = TYPE_UNION;
     type->size = 0;
     for ( size_t i = 0; i < num_fields - 1; i++ ){
          type->alignment = MAX(fields[i].type->alignment, fields[i+1].type->alignment);
          type->size = MAX(type->size, fields[i].type->size);
     }
     type->size = ALIGN_UP(MAX(type->size,fields[num_fields-1].type->size), type->alignment);
     type->aggregate.fields = arena_dup(&resolve_arena,fields,sizeof(*fields)*num_fields);
     type->aggregate.num_fields = num_fields;
}
void complete_type(Type *type){
     if ( type->kind == TYPE_COMPLETING ){
          fatal("Mad cycles detected!!\n");
     } else if ( type->kind != TYPE_INCOMPLETE ){
          return;
     }
     assert( type->sym->decl->kind == DECL_STRUCT || type->sym->decl->kind == DECL_UNION );
     aggregate_item *items = type->sym->decl->aggregate_decl.aggregate_items;
     size_t num_items = type->sym->decl->aggregate_decl.num_aggregate_items;
     type->kind = TYPE_COMPLETING;
     TypeField *fields = NULL ;
     TypeField temp_field = {};
     Type *tmp = NULL;
     for ( aggregate_item *it = items; it != items+num_items; it++ ){
         tmp = resolve_typespec(it->type); 
         complete_type(tmp);
         for  ( size_t j = 0; j < it->num_names; j ++ ){
              temp_field.type = tmp; temp_field.name = it->name_list[j];
              buff_push(fields,temp_field);
         }
     }

     if ( type->sym->decl->kind == DECL_STRUCT ){
          complete_type_struct(type,fields,buff_len(fields));
     } else {
          assert(type->sym->decl->kind = DECL_UNION );
          complete_type_union(type,fields,buff_len(fields));
     }
     duplicate_field_check(type);            
     if ( is_local_scope ){
          sym_add_local(type->sym);
     }  else {
          buff_push(ordered_syms,type->sym);
     }
}

int64_t resolve_int_const_expr(Expr *expr){
     ResolvedExpr new = resolve_expr(expr);
     if ( !new.is_const ){
          resolve_error(&expr->location,"Expected constant expression");
     }
     return new.int_val;
}

Type *resolve_typespec(TypeSpec *typespec){
     if ( !typespec ){
          return type_void;
     }
     switch ( typespec->kind ){

          case TYPESPEC_NAME:{
               Sym *sym = resolve_name(typespec->name); 
               if ( sym->kind != SYM_TYPE ){
                    fatal("%s is not a type!!\n",typespec->name);
                    return NULL;
               }
               return sym->type;
	          break;
          }
          case TYPESPEC_FUNC:{
               Type **args = NULL;
               for ( TypeSpec **it = typespec->func.args; it != typespec->func.args + typespec->func.num_args; it++ ){
                   buff_push( args,resolve_typespec(*it) ); 
               }
               Type *ret_type = resolve_typespec(typespec->func.ret_type);
               return type_func(args,typespec->func.num_args,ret_type);
	          break;
          }
          case TYPESPEC_ARRAY:{
               Type *type = resolve_typespec(typespec->array.base_type);
               complete_type(type);
               return type_array(type, resolve_int_const_expr(typespec->array.size));
	          break;
          }
          case TYPESPEC_POINTER:
               return type_ptr(resolve_typespec(typespec->ptr.base_type));
               break;
          default: 
               assert(0);
               break;
     }
}


#define is_assign(x) ( (x) >= TOKEN_ASSIGN && (x) <= TOKEN_RSHIFT_ASSIGN )

void resolve_stmt_decl(Decl *decl){
     extern Type *resolve_const_decl(Decl *, int64_t *);
     extern Type *resolve_var_decl(Decl *);
     extern Type *resolve_type_decl(Decl *);
     Type *type = NULL;
     if ( in_local(decl->name) ){
          resolve_error(&decl->location,"Duplicate name in the same scope!\n");
     }
     Sym *sym = new_sym(SYM_UNRESOLVED,decl->name,decl); 
     switch ( decl->kind ){
          case DECL_CONST:{
               sym->kind = SYM_CONST;
               type = resolve_const_decl(decl,&sym->val);
               break;
          }
          case DECL_VAR:
               sym->kind = SYM_VAR;
               type = resolve_var_decl(decl);
               break;
          case DECL_TYPEDEF:
               sym->kind = SYM_TYPE;
               type = resolve_type_decl(decl);
               break;
          case DECL_ENUM:{
               Sym *tmp;
               type = type_enum;
               for ( size_t i = 0; i < decl->enum_decl.num_enum_items; i++ ){
                    tmp = new_sym(SYM_ENUM_CONST,decl->enum_decl.enum_items[i].name,decl);
                    sym_add_local(tmp);
               }
          }
          case DECL_STRUCT:
          case DECL_UNION:{
               sym->kind = SYM_TYPE;
               type = type_incomplete(sym);
               break;
          }
          default:
               assert(0);
               break;
     }
     sym->state = SYM_RESOLVED;
     sym->type = type;
     decl->sym = sym;
     sym_add_local(sym);
}
void resolve_cond_expr(Expr *cond_expr){
     ResolvedExpr new = resolve_expr(cond_expr);
     if ( new.type != type_int  && new.type->kind != TYPE_PTR ){
          resolve_error(&cond_expr->location,"Value of conditional expression is not an integer or a pointer type!\n");
     }
}
void resolve_stmt(Stmt *stmt, Type *ret_type){
     extern void resolve_stmt_block(StmtBlock , Type *);
     switch ( stmt->kind ){
          case STMT_INIT:{
               ResolvedExpr new = resolve_expr(stmt->init_stmt.rhs);
               Sym *sym =sym_var(stmt->init_stmt.name,new.type); 
               sym_add_local(sym);
               stmt->init_stmt.sym = sym;
			break;
          }
          case STMT_EXPR:
               resolve_expr(stmt->expr_stmt); 
			break;
          case STMT_FOR:{
               if ( stmt->for_stmt.stmt_init){
                    resolve_stmt(stmt->for_stmt.stmt_init,ret_type);
               }
               if ( stmt->for_stmt.expr_cond ){
                    resolve_cond_expr( stmt->for_stmt.expr_cond );
               }
               if ( stmt->for_stmt.stmt_update ){
                    resolve_stmt(stmt->for_stmt.stmt_update , ret_type );
               }
               resolve_stmt_block(stmt->for_stmt.block,ret_type);
			break;
          }
          case STMT_WHILE:
          case STMT_DO_WHILE:
               resolve_cond_expr(stmt->while_stmt.expr_cond);
               resolve_stmt_block(stmt->while_stmt.block,ret_type);
			break;
          case STMT_IF:{
               resolve_cond_expr(stmt->if_stmt.cond);
               resolve_stmt_block(stmt->if_stmt.if_block,ret_type);
               for ( size_t i = 0; i < stmt->if_stmt.num_elseifs; i++ ){
                    resolve_cond_expr(stmt->if_stmt.elseifs[i].cond);
                    resolve_stmt_block(stmt->if_stmt.elseifs[i].elseif_block,ret_type);
               }
               if ( stmt->if_stmt.else_block.num_stmts ){
                    resolve_stmt_block(stmt->if_stmt.else_block,ret_type);
               } 
			break;
          }
          case STMT_SWITCH:{
               resolve_cond_expr(stmt->switch_stmt.expr);
               for ( size_t i = 0; i < stmt->switch_stmt.num_cases; i++ ){
                    Case tmp = stmt->switch_stmt.cases[i];
                    for ( size_t j = 0; j < tmp.num_expr; j++ ){
                         ResolvedExpr new = resolve_expr(tmp.expr_list[j]);
                         if ( new.type!=type_int || !new.is_const ){
                              resolve_error(&stmt->location,"Case expression with a non integer or a non constant expression!\n");
                         }
                    }
                    resolve_stmt_block(tmp.case_block,ret_type);
               }                
			break;
          }
          case STMT_RETURN:{
               ResolvedExpr new = { .type = type_void };
               if ( stmt->return_stmt.return_expr ){
                    new = resolve_expr_expected(stmt->return_stmt.return_expr,ret_type);
               }
               if ( new.type != ret_type ){
                    resolve_error(&stmt->location,"Type of return expression doesn't match with function return type\n");
               } 
			break;
          }
          case STMT_BREAK:
          case STMT_CONTINUE:
			break;
          case STMT_BLOCK:
               resolve_stmt_block(stmt->block,ret_type);
			break;
          case STMT_DECL:{
               resolve_stmt_decl(stmt->decl_stmt);
               break;
          }

     }     
}

void resolve_stmt_block(StmtBlock block, Type *ret_type){
     is_local_scope = true;
     SymNode **scope_start = sym_enter();
     for ( size_t i = 0; i < block.num_stmts; i++ ){
          resolve_stmt( block.stmts[i], ret_type );
     }
     sym_exit(scope_start);
}

void resolve_func(Sym *sym ){
     is_local_scope = true;
     SymNode **syms = sym_enter();
     Decl *decl = sym->decl;
     assert(sym->kind == SYM_FUNC );
     assert(sym->state = SYM_RESOLVED );
     Type *type = sym->type; 
     for ( size_t i = 0 ; i < type->func.num_params; i++ ){
          func_param tmp = decl->func_decl.param_list[i];
          sym_add_local(sym_var(tmp.name,type->func.param_list[i]));     
          assert( resolve_name(tmp.name)->type == type->func.param_list[i]);
     }
     StmtBlock block = decl->func_decl.block;
     for ( size_t i = 0; i < block.num_stmts; i++ ){
          resolve_stmt( block.stmts[i], sym->type->func.ret_type );
     }
     sym_exit(syms);
     is_local_scope = false;
}

Type *resolve_func_decl(Decl *decl){
     assert(decl->kind == DECL_FUNC);
     Type **param_list = NULL;
     Type *tmp = NULL;     
     for ( size_t i = 0; i < decl->func_decl.num_params; i++ ){
          tmp = resolve_typespec(decl->func_decl.param_list[i].type);
          complete_type(tmp);
          buff_push(param_list,tmp);
     }
     if ( decl->func_decl.ret_type ){
          tmp = resolve_typespec(decl->func_decl.ret_type);
     } else {
          tmp = type_void;
     }
     complete_type(tmp);
     tmp = type_func(param_list,decl->func_decl.num_params,tmp);
     return tmp;
}

Type *resolve_const_decl(Decl *decl, int64_t *val){
     assert(decl->kind == DECL_CONST);
     ResolvedExpr x = resolve_expr(decl->const_decl.expr);
     if ( x.is_const && !x.is_lvalue && x.type == type_int){
          *val = x.int_val;
     } else {
          resolve_error(&decl->location,"Initializer for a const decl is not a constant expression!\n");
     }
     return x.type;
}


Type *resolve_var_decl(Decl *decl){
     assert(decl->kind == DECL_VAR );
     Type *type = NULL;
     if  ( decl->var_decl.type ){
          type = resolve_typespec(decl->var_decl.type);
     }
     if ( type == type_void ){
          resolve_error(&decl->location,"Cant have void data types!\n");
     }
     if ( decl->var_decl.expr ){
          ResolvedExpr result = resolve_expr_expected(decl->var_decl.expr,type);
          if ( type && result.type != type ){
               resolve_error(&decl->location,"Expression type dosen't match the variable type!\n");
          }
          type = result.type;
     }
     assert(type!=NULL);
     complete_type(type);
     return type;
}


Type *resolve_type_decl(Decl *decl){
     if (decl->kind == DECL_TYPEDEF){
          return resolve_typespec(decl->typedef_decl.type);
     } else if ( decl->kind = DECL_ENUM ){
          return type_int;
     }
}

void *resolve_sym(Sym *sym){

     /*
      * Structs and union are treated as incomplete types so they are taken as resolved types.
      * because we don't need definition of structs when using as pointers.
      * Their types are completed when the details of struct are need.
      * So the only unresolved sym type is the typedef
      */
     if ( sym->state == SYM_RESOLVED ){
          return sym;
     } else if ( sym->state == SYM_RESOLVING ){
          fatal("Cyclic dependency of varaibles!!\n");
     }
     assert(sym->state == SYM_UNRESOLVED);
     sym->state = SYM_RESOLVING;
     
     switch ( sym->kind ){
         case SYM_CONST:
              sym->type = resolve_const_decl(sym->decl,&sym->val);
              break;
         case SYM_VAR:
              sym->type = resolve_var_decl(sym->decl);
              break;
         case SYM_TYPE:
              sym->type = resolve_type_decl(sym->decl);
              break;
         case SYM_FUNC:
              sym->type = resolve_func_decl(sym->decl);
              break;
          case SYM_ENUM_CONST:
               break;
         default:
               assert(0);
               break;
     }
     sym->state = SYM_RESOLVED;
     sym->decl->sym  = sym;
     buff_push(ordered_syms,sym);
}

ResolvedExpr resolve_null;
ResolvedExpr resolve_lvalue(Type *type){
     ResolvedExpr new = {};
     new.type = type;
     new.is_lvalue = true;
     new.is_const = false;
     return new;
}

ResolvedExpr resolve_rvalue(Type *type){
     ResolvedExpr new = {};
     new.type = type;
     new.is_lvalue = false;
     new.is_const = false;
     return new;
}

ResolvedExpr resolve_const_int(int64_t int_val){
     ResolvedExpr new = {};
     new.type = type_int;
     new.is_const = true;
     new.is_lvalue = false;
     new.int_val = int_val;
     return new;
}

ResolvedExpr resolve_const_float(double float_val){
     ResolvedExpr new = {};
     new.type = type_int;
     new.is_const = true;
     new.is_lvalue = false;
     new.float_val = float_val;
     return new;
}

ResolvedExpr resolve_expr_name(const char *name){
     
     Sym *new = sym_get(name);
     if ( !new ){
          fatal("identifier %s is not defined\n",name);
     }
     resolve_sym(new);
     if ( new->kind == SYM_VAR ){
          return resolve_lvalue(new->type);
     } else if ( new->kind == SYM_CONST ){
          return resolve_const_int(new->val);
     } else if ( new->kind == SYM_FUNC){
          return resolve_rvalue(new->type);
     } else if ( new->kind == SYM_ENUM_CONST ){
          return resolve_rvalue(new->type);
     } else {
          fatal("name should be a var or a constant\n");
     }
     return resolve_null;
}

ResolvedExpr resolve_expr_field(Expr *expr){

     ResolvedExpr left = resolve_expr(expr->field_expr.operand);
     Type *type  = left.type;
     complete_type(type);
     if ( type->kind != TYPE_STRUCT &&  type->kind != TYPE_UNION ){
          resolve_error(&expr->location,"field access must be on a struct or a pointer\n");
     }
          
     for ( size_t i = 0; i < type->aggregate.num_fields; i++ ){
          if ( expr->field_expr.field_name == type->aggregate.fields[i].name ){
             return  left.is_lvalue?resolve_lvalue(type->aggregate.fields[i].type):resolve_rvalue(type->aggregate.fields[i].type);
          }
     }
     resolve_error(&expr->location,"Invalid member %s for struct %s\n",expr->field_expr.field_name,type->sym->decl->name);
     return resolve_null;
}

int64_t eval_expr_unary(int64_t val, TokenKind op){
     switch ( op ){
     	case TOKEN_NOT:
               return !val;
               break;
     	case TOKEN_COMPLEMENT:
               return ~val;
               break;
          case TOKEN_ADD:
               return +val;
               break;
          case TOKEN_SUB:
               return -val;
               break;
          default:
               fatal("Unidentified unary op!\n");
               break;
     }
     return 0;
}


Type *ptr_decay(Type *type){
     /*
      * Chages array data type to its corresponding pointer type
      * other data types remain unchanged
      */
     if ( type->kind == TYPE_ARRAY ){
          return type_ptr(type->array.base_type);
     } else {
          return type;
     }
}

ResolvedExpr resolve_expr_unary(Expr *expr){
     ResolvedExpr new= resolve_expr(expr->unary_expr.operand);
     Type *type = new.type;
     switch ( expr->unary_expr.op ){
          case TOKEN_MUL:{
               type = ptr_decay(type);
               if ( type->kind != TYPE_PTR ){
                    resolve_error(&expr->location,"Trying to de-reference a non pointer data type!\n");
               }
               return resolve_lvalue(type->ptr.base_type);
               break;
          }
          case TOKEN_BAND:
               if ( !new.is_lvalue ){
                    resolve_error(&expr->location,"Trying to reference a non lvalue expression!\n");
               }
               return resolve_rvalue(type_ptr(type));
               break;
          default:{
               if ( new.is_const ){
                    return resolve_const_int(eval_expr_unary(new.int_val,expr->unary_expr.op));
               } else {
                    return resolve_rvalue(type);
               }
               break;
          }
     } 
}

int64_t eval_expr_binary(int64_t left, int64_t right,TokenKind op){
     switch ( op ){
     	case TOKEN_MUL:
               return left*right;
			break;
     	case TOKEN_MOD:
               if ( right == 0 ){
                    fatal("Attempting division by 0\n");
               }
               return left % right;
			break;
     	case TOKEN_DIV:
               if ( right == 0 ){
                    fatal("Attempting division by 0\n");
               }
               return left / right;
			break;
     	case TOKEN_BAND:
               return left & right;
			break;
     	case TOKEN_LSHIFT:
               return left << right;
			break;
     	case TOKEN_RSHIFT:
               return left >> right;
			break;
     	case TOKEN_ADD:
               return left + right;
			break;
     	case TOKEN_XOR:
               return left ^ right;
			break;
     	case TOKEN_SUB:
               return left - right;
			break;
     	case TOKEN_BOR:
               return left|right;
			break;
     	case TOKEN_EQ:
               return left == right;
			break;
     	case TOKEN_NOTEQ:
               return left != right;
			break;
     	case TOKEN_LTEQ:
               return left <= right;
			break;
     	case TOKEN_GTEQ:
               return left>=right;
			break;
     	case TOKEN_AND:
               return left && right;
			break;
     	case TOKEN_OR:
               return left || right;
			break;
     	case TOKEN_LT:
               return left < right;
			break;
     	case TOKEN_GT:
               return left > right;
			break;
          default:
               assert(0);
               break;
     }
}

ResolvedExpr resolve_expr_binary(Expr *expr){
     ResolvedExpr left = resolve_expr(expr->binary_expr.left);
     ResolvedExpr right = {} ;
     if  ( is_assign(expr->binary_expr.op) ){
          if ( !left.is_lvalue || left.is_const ){
               resolve_error(&expr->location,"Attempt to assign to a non lvalue or constant  type!\n");
          }
     } 
     if ( expr->binary_expr.op == TOKEN_INC || expr->binary_expr.op == TOKEN_DEC ){
          right.type = type_int;right.int_val = 1; right.is_const = true; right.is_lvalue = true ;
     } else {
          right = resolve_expr(expr->binary_expr.right);
     }
     if ( left.type != right.type ){
          resolve_error(&expr->location,"Type of values do not match\n"); // TODO
     }
     if ( left.is_const && right.is_const ){
          return resolve_const_int(eval_expr_binary(left.int_val,right.int_val,expr->binary_expr.op));
     } else {
          return resolve_rvalue(left.type);
     }
}

ResolvedExpr resolve_expr_compound(Expr *expr, Type *expected_type){
     Type *comp_type = NULL;
     if ( !expected_type && !expr->compound_expr.type ){
          resolve_error(&expr->location,"Compound Expr with no designated type!\n");
     } else if ( expr->compound_expr.type ) {
          comp_type = resolve_typespec(expr->compound_expr.type);
          if ( expected_type && comp_type  != expected_type ){
               resolve_error(&expr->location,"Expression type does not match the designated type!\n");
          }
     } 
     
     Type *type = expected_type? expected_type : resolve_typespec(expr->compound_expr.type);
     complete_type(type);
     
     //assert( type->kind == TYPE_STRUCT ); 
     if ( type->kind != TYPE_STRUCT && type->kind != TYPE_UNION && type->kind != TYPE_ARRAY ){
          resolve_error(&expr->location,"Attempt to use compound expression on a non-compound type\n");
     }
     if ( type->kind == TYPE_STRUCT || type->kind == TYPE_UNION){
          if ( expr->compound_expr.num_args > type->aggregate.num_fields ){
               resolve_error(&expr->location,"Compound expr has more members than the type\n");
          }
          ResolvedExpr new = {};
          size_t currentIndex = 0;
          for ( size_t i = 0; i < expr->compound_expr.num_args; i++ ){
               CompoundField tmp = expr->compound_expr.fields[i];
               if ( tmp.kind == FIELD_INDEX ){
                    resolve_error(&expr->location,"Attempt to use index initializer in a non array data type!\n");
               } else if ( tmp.kind == FIELD_NONE ){
                    assert(!tmp.field_expr);
                    if ( currentIndex > type->aggregate.num_fields - 1 ){
                         resolve_error(&expr->location,"Excess initializer for struct \n");
                    }
                    new = resolve_expr_expected(tmp.expr,type->aggregate.fields[currentIndex].type);
                    if ( new.type != type->aggregate.fields[currentIndex].type ){
                         resolve_error(&expr->location,"Expression and field type do not match!\n");
                    }
                    currentIndex++;
               } else {
                    assert( tmp.kind == FIELD_NAME );
                    if ( tmp.field_expr->kind != EXPR_NAME ){
                         resolve_error(&expr->location,"Initializer is not a name \n");
                    }
                    Type *field_type = NULL;
                    for ( size_t z = 0; z < type->aggregate.num_fields; z++ ){
                         if ( tmp.field_expr->name == type->aggregate.fields[z].name ){
                              currentIndex = z;
                              field_type = type->aggregate.fields[z].type;
                              break;
                         }
                    }
                    if ( !field_type ){
                        resolve_error(&expr->location,"Field %s  does not exist in the struct.\n",tmp.field_expr->name);
                    }
                    new = resolve_expr_expected(tmp.expr,field_type); 
                    if ( new.type != field_type ){
                         resolve_error(&expr->location,"Expression and field type do not match!\n");
                    }
                    currentIndex++;
               } 
          }
     } else {
          assert(type->kind == TYPE_ARRAY );
          if ( expr->compound_expr.num_args > type->array.size ){
               resolve_error(&expr->location,"Array expression has more members than the array length\n");
          }
          ResolvedExpr new = {};
          ResolvedExpr resolved_field_expr = {};
          size_t currentIndex = 0;
          for ( size_t i = 0; i < expr->compound_expr.num_args; i++ ){
               CompoundField tmp = expr->compound_expr.fields[i];
               if ( tmp.kind == FIELD_INDEX ){
                    if (!tmp.field_expr){
                         resolve_error(&expr->location,"Can't have empty field expressions!!\n");
                    }
                    resolved_field_expr = resolve_expr(tmp.field_expr); 
                    if ( resolved_field_expr.type != type_int ){
                         resolve_error(&expr->location,"Index expressions must be of type int\n");
                    } else if ( !resolved_field_expr.is_const ){
                         resolve_error(&expr->location,"Iniializer expression for index must be a constant.\n");
                    }
                    assert(resolved_field_expr.is_const);
                    if ( resolved_field_expr.int_val < 0 ){
                         resolve_error(&expr->location,"Can\'t have negative indices in array!\n");
                    } else {
                         currentIndex =  resolved_field_expr.int_val;
                         if ( currentIndex > type->array.size ){
                              resolve_error(&expr->location,"Index value exceed the array bounds!\n");
                         }
                    } 
                    currentIndex++;
               }  else if ( tmp.kind == FIELD_NAME ){
                    resolve_error(&expr->location,"Attempt to use field initializer in a non struct or union type!\n");
               }else {
                    assert(tmp.kind == FIELD_NONE );
                    if ( currentIndex > type->array.size - 1 ){
                         resolve_error(&expr->location,"Initializing out of bound!\n");
                         
                    }
                    currentIndex++;
               }
               new = resolve_expr_expected(tmp.expr,type->array.base_type);
               if ( new.type != type->array.base_type ){
                    resolve_error(&expr->location,"Type mismatch with array type\n");
               }
               
          }
     } 
     expr->compound_expr.resolved_type = type;
     return resolve_rvalue(type);
}


ResolvedExpr resolve_expr_call(Expr *expr,Type *expected_type){
     assert(expr->kind == EXPR_CALL);
     ResolvedExpr new = resolve_expr(expr->func_call_expr.operand);
     Type *type = new.type;
     if ( type->kind != TYPE_FUNC ){
          resolve_error(&expr->location,"Undeclared function being called!\n");
     } 
     ResolvedExpr result = {};
     if ( type->func.num_params != expr->func_call_expr.num_args ){
          resolve_error(&expr->location,"Number of arguments of function call dosen't match with declaration!\n");
     } 
     for ( size_t i = 0; i < type->func.num_params; i++ ){
          result = resolve_expr_expected(expr->func_call_expr.args[i],type->func.param_list[i]);
          if ( result.type != type->func.param_list[i] ){
               resolve_error(&expr->location,"Function Argument type mismtach \n");
          }
     }
     return resolve_rvalue(type->func.ret_type); 
}

ResolvedExpr resolve_expr(Expr *expr){
     return resolve_expr_expected(expr,NULL);
}

ResolvedExpr resolve_expr_ternary(Expr *expr){
     assert(expr->kind == EXPR_TERNARY);
     ResolvedExpr cond_expr = resolve_expr(expr->ternary_expr.cond_expr);
     ResolvedExpr then_expr = resolve_expr(expr->ternary_expr.then_expr);
     ResolvedExpr else_expr = resolve_expr(expr->ternary_expr.else_expr);
     if ( cond_expr.type->kind != TYPE_INT && cond_expr.type->kind != TYPE_PTR ){
          resolve_error(&expr->location,"The value of condition expr is not an integer!\n");
     }
     if ( then_expr.type != else_expr.type ){
          resolve_error(&expr->location,"Type mismatch for condition expressions of ternary expression!\n");
     }
     if ( cond_expr.is_const && then_expr.is_const && else_expr.is_const ){
         return cond_expr.int_val?then_expr:else_expr; 
     }
     return resolve_rvalue(then_expr.type);
}

ResolvedExpr resolve_expr_cast(Expr *expr){
     // TODO: add more cast types
     assert(expr->kind == EXPR_CAST);
     ResolvedExpr new = resolve_expr(expr->cast_expr.expr);
     Type *cast_type = resolve_typespec(expr->cast_expr.cast_type);
     complete_type(cast_type);
     if ( cast_type->kind == TYPE_PTR ){
          if ( new.type->kind != TYPE_PTR && new.type->kind != TYPE_INT ){
               resolve_error(&expr->location,"Invalid expression to cast to pointer type.\n");
          }
     } else if ( cast_type->kind == TYPE_INT ){
          if ( new.type->kind != TYPE_PTR && new.type->kind != TYPE_INT ){
               resolve_error(&expr->location,"Invalid expression to cast to integer type.\n");
          }
     } else {
          resolve_error(&expr->location,"Invalid cast type!\n");
     }
     expr->cast_expr.resolved_type = cast_type;
     return resolve_rvalue(cast_type);
}

ResolvedExpr resolve_expr_index(Expr *expr){
     assert(expr->kind == EXPR_INDEX);
     ResolvedExpr new = resolve_expr(expr->array_expr.operand);
     ResolvedExpr index = resolve_expr(expr->array_expr.index);
     if ( new.type->kind != TYPE_ARRAY && new.type->kind != TYPE_PTR ){
          resolve_error(&expr->location,"Attempting to index a non array and non pointer data type\n");
     }
     if ( index.type->kind != TYPE_INT ){
          resolve_error(&expr->location,"Array indices must be an integer!\n");
     }
     return resolve_lvalue(new.type->array.base_type);
}
ResolvedExpr resolve_expr_expected(Expr *expr,Type *expected_type){
     switch (expr->kind){
          case EXPR_INT:
               return resolve_const_int(expr->int_val);
               break;
          case EXPR_FLOAT:
               return resolve_const_float(expr->float_val);
               break;
          case EXPR_STR:{
               Type *type = type_ptr(type_char);
               return resolve_rvalue(type);
               break;
          }
          case EXPR_NAME:
               return resolve_expr_name(expr->name);
               break;
          case EXPR_CALL:
               return resolve_expr_call(expr,NULL);
               break;
          case EXPR_CAST:
               return resolve_expr_cast(expr);
               break;
          case EXPR_SIZEOF_EXPR:{
               ResolvedExpr new = resolve_expr_expected(expr->sizeof_expr,expected_type);
               Type *t= new.type;
               assert(t!=NULL);
               complete_type(t);
               return resolve_const_int(t->size);
               break;
          }
          case EXPR_SIZEOF_TYPE:{
               Type *type = resolve_typespec(expr->sizeof_type.type);
               complete_type(type);
               expr->sizeof_type.resolved_type = type;
               return resolve_const_int(type->size);
               break;
          }
          case EXPR_UNARY:
               return resolve_expr_unary(expr);
               break;
          case EXPR_BINARY:
               return resolve_expr_binary(expr);
               break;
          case EXPR_TERNARY:
               return resolve_expr_ternary(expr);
               break;
          case EXPR_INDEX:
               return resolve_expr_index(expr);
               break;
          case EXPR_COMPOUND:
               return resolve_expr_compound(expr,expected_type);
               break;
          case EXPR_FIELD:
               return resolve_expr_field(expr);
               break;
          default:
               assert(0);
               break;
     }
}



void type_test(void){
     Type *int_ptr1  = type_ptr(type_int) ;
     Type *int_ptr2  = type_ptr(type_int) ;

     Type *float_ptr1  = type_ptr(type_float) ;
     Type *float_ptr2  = type_ptr(type_float) ;
     assert(int_ptr1 == int_ptr2);
     assert(int_ptr1 != float_ptr1 );
     assert(float_ptr1 == float_ptr2);

     Type *int_array1 = type_array(type_int,10);
     Type *int_array2 = type_array(type_int,10);
     assert(int_array1 == int_array2);
     Type *int_array3 = type_array(type_int,11);
     assert( int_array1 != int_array3);
     Type *float_array1= type_array(type_float,10);
     assert(int_array1!=float_array1);

     Type *int_ptr_array = type_array(int_ptr1,10);
     Type *int_ptr_array1 = type_array(int_ptr1,10);
     assert(int_ptr_array == int_ptr_array1);
     Type *list[] = {
          int_array1,
          int_ptr1,
          type_int
     };
     Type *func1 = type_func(list,sizeof(list)/sizeof(*list),type_int);
     Type *func2 = type_func(list,sizeof(list)/sizeof(*list),type_int);
     assert(func1 == func2 );

}

void sym_add_type(const char *name,Type *type){
     Sym *new = NULL;
     new = new_sym(SYM_TYPE,name,NULL); 
     new->state = SYM_RESOLVED;
     new->type = type;
     buff_push(global_sym_list,new);
}


void resolve_test(void){
     init_intern_keyword();
     sym_add_type(str_intern("int"),type_int);
     sym_add_type(str_intern("float"),type_float);
     sym_add_type(str_intern( "char" ),type_char);
     sym_add_type(str_intern( "void" ),type_void);
     /*char *list[] = {
          "var x : Vector = {1,2};",
          "struct Vector { x,y:int ; };",
     };*/
     char *list[] = { 
//        "struct Vector { x, y: int; }",
 //       "var i: int",
//        "func f1() { v := Vector{1, 2}; j := i; i++; j++; v.x = 2*j; }",
//        "func f2(n: int): int { struct S { x:int; y :int; }; var x : S = { 1, 2};return 2*n; }",
//      "var x:int**;",
 //       "func f3(x: int): int { if (x) { var x:int= 2; return -x; } else if (x % 2 == 0) { return 42; } else { var x:int= 2;return -1; } }",
//        "func f4(n: int): int { for (i := 0; i < n; i++) { if (i % 3 == 0) { return n; } } return 0; }",
 //       "func f5(x: int): int { switch(x) { case 0: case 1: return 42; case 3:default: return -1; } }",
//        "func f6(n: int): int { p := 1; while (n) { p *= 2; n--; } return p; }",
//        "func f7(n: int): int { var p:int = 1;do { p *= 2; n--; } while (n); return p; }",
//        "func add(v: Vector, w: Vector): Vector { return {v.x + w.x, v.y + w.y}; }",
          
  //        "const u = 2;",
 //         "func fact(x:int):int{var i:int = u ;product := 1; while(x!=1){product*=x;x = x - 1 ;} var e : int = i;return x;}",
          
          /*"struct notVector { x :int ;};",
          "func addV(x:Vector,y:Vector):Vector { return {p.x+q.x,p.y+q.y}; }",
          "var v : Vector = addV( {1,2,3},{1,2} );",
          "struct values { x:int; z : Vector;};",
          "var x:Vector[3] = { [1] = {1,2},[2]= {3,4} };" ,
          "var j : Vector[2][2] = { { {1,2},{3,4} }, { {4,5}, {5,6} } };",
          "var k : int[8] = { [2] = 9, [7] = 7,[1] = 0};",
         "struct Vector3 {x,y,z:int;};",
         "struct tf {x:int;x:float;};",
          "var w : int *;",
          "var r : Vector3 = { y = 2,3};",
        "func foo(x:int,y:int){return;};",
          "const i = 2 + 3;",
          "var a : int[3] = {1,2,3};",
          "var x : int = *a;",
          "var o : int = cast(int)1+2;",
          "const t = 2*3 + sizeof(x)/2 + ~1;",
          "var m:char = \'a\';",
//          "var h = \"fuck this shit\";",
//          "var pp = !a[0];"   
//          "const x = +1;",
//          "const s = ~-1;",
//          "var p:int;",
//          "var b = *p;",
//          "var x = cast(int)1;",
//          "var n:int = 1;",
 //         "const x = 1 ? n : 3;",
 //         "func addV(x:Vector,y:Vector):Vector { return {p.x+q.x,p.y+q.y}; }",
//         "func add(x:int,y:int):int{ return x+y;}",
//          "var z:int = add(1,2);",
//          "var c:int[3] = {1,2,3};",
//          "var e:int* = &p",
//          "var p:int = 42",
//          "var q:Vector = addV({1,2},{3,4});",
//          "var f:int[2] = (:int[2]){123,342};",
//          "var z:Vector = Vector{x,2,3,4};",
//
//          "const n = 1+sizeof(p);",
//          "const g = sizeof(:T*)",
//          "var p:T*;",
//          "var u = *p;",
//          "struct T { a:int[n]; };",
//          "var r = &t.a;",
//          "var t:T;",
//          "typedef S = int[n+m];",
//          "const m = sizeof(t.a);",
//          "var i = n + m;",
//          "var q = &i;"*/
     };
     Decl *tmp_decl;
     for( char **it = list; it != list + sizeof(list)/sizeof(char *); it++ ){
          init_stream(*it);
          tmp_decl = parse_decl();
          sym_add_decl(tmp_decl);
     }
    for ( Sym **it = global_sym_list; it != buff_end(global_sym_list); it++ ){
          resolve_sym(*it);
          if ( (*it)->kind == SYM_FUNC ){
               resolve_func(*it);
          }
          complete_type((*it)->type);
     }

     for ( Sym **it = ordered_syms; it != buff_end(ordered_syms); it++ ){
          print_decl( (*it)->decl );
          printf("\nAlignment: %zu, Size: %zu\n", (*it)->type->alignment,(*it)->type->size); 
          printf("\n");
     }
     buff_free(global_sym_list);
     buff_free(ordered_syms);
}
