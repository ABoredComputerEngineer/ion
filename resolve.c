#define INT_SIZE 4
#define FLOAT_SIZE 4
#define PTR_SIZE 8

Arena resolve_arena;
typedef struct Type Type;
typedef struct Entity Entity;
typedef enum TypeKind {
     TYPE_NONE,
     TYPE_INCOMPLETE,
     TYPE_COMPLETING,
     TYPE_ENUM,
     TYPE_INT,
     TYPE_FLOAT,
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
     Entity *entity;
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

typedef enum EntityKind {
     ENTITY_NONE,
     ENTITY_CONST,
     ENTITY_VAR,
     ENTITY_FUNC,
     ENTITY_TYPE,
     ENTITY_ENUM_CONST
} EntityKind;

typedef enum EntityState {
     ENTITY_UNRESOLVED,
     ENTITY_RESOLVING,
     ENTITY_RESOLVED
} EntityState;


struct Entity {
     const char *name;
     EntityKind kind;
     EntityState state;
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
     new->array.size = size;
     CacheArrayType tmp = { new,base_type,size };
     buff_push(array_list,tmp);
     return new;
}

Type *type_incomplete(Entity *entity){
     Type *new = type_alloc(TYPE_INCOMPLETE);
     new->entity = entity;
     return new;
}

Type INT_TYPE = { TYPE_INT, INT_SIZE};
Type FLOAT_TYPE = {TYPE_FLOAT, FLOAT_SIZE};
Type *type_int = &INT_TYPE;
Type *type_float = &FLOAT_TYPE;

Entity **entity_list = NULL;
Entity *entity_get(const char *);
Entity *new_entity(EntityKind kind, const char *name, Decl *decl){
     Entity *new = arena_alloc(&resolve_arena,sizeof(Entity));
     new->kind = kind;
     new->state = ENTITY_UNRESOLVED;
     new->name = name;
     new->decl = decl;
     new->type = NULL;
     return new;
}


Entity *entity_add_decl(Decl *decl){
     if ( entity_get(decl->name) ){
          fatal("Duplicate var names, %s\n",decl->name);
          assert(0);
     }
     EntityKind kind = ENTITY_NONE;
     switch ( decl->kind ){
          case DECL_ENUM:
          case DECL_TYPEDEF:
          case DECL_STRUCT:
          case DECL_UNION:
               kind = ENTITY_TYPE;
               break;
          case DECL_CONST:
               kind = ENTITY_CONST;
               break;
          case DECL_VAR:
               kind = ENTITY_VAR;
               break;
          case DECL_FUNC:
               kind = ENTITY_FUNC;
               break;
          default:
               assert(0);
               break;
     }
     Entity *new = new_entity(kind,decl->name,decl);
     if ( decl->kind == DECL_STRUCT || decl->kind == DECL_UNION ){
          /*
           * For struct and union we use an incomplete type to denote it.
           * The type gets completed if the declaration is complete.
           * e.g. 'struct type' is an incomplete type
           *
           */
          new->type = type_incomplete(new);
          new->state = ENTITY_RESOLVED;  
     }
     buff_push(entity_list,new);
     if  ( decl->kind == DECL_ENUM ){
          Entity *tmp;
          for ( size_t i = 0; i < decl->enum_decl.num_enum_items; i++ ){
              tmp = new_entity(ENTITY_ENUM_CONST,decl->enum_decl.enum_items[i].name,decl);
              buff_push(entity_list,tmp); 
          }
     }
     return new;

}
Type *resolve_typespec(TypeSpec *);
Entity *entity_get(const char *name){
     for ( Entity **it = entity_list; it != buff_end(entity_list); it++ ){
          if ( (*it)->name == name ){
               return *it;
          }
     } 
     return NULL;
}

Entity *resolve_name(const char *name){
     Entity *new = entity_get(name);
     if ( !new ){
          fatal("%s is not defined!!\n",name);
     }
     return new;
}

Entity **ordered_entities = NULL;
ResolvedExpr resolved_null = {};
ResolvedExpr resolve_expr(Expr *);
ResolvedExpr resolve_expr_expected(Expr *,Type *);
void complete_type_struct(Type *type,TypeField *fields, size_t num_fields){
     assert(type->kind == TYPE_COMPLETING);
     type->kind = TYPE_STRUCT;
     type->size = 0;
     for ( size_t i = 0; i < num_fields; i++ ){
          //   TODO : Alignment
          type->size += fields[i].type->size;
     }
     type->aggregate.fields = arena_dup(&resolve_arena,fields,sizeof(*fields)*num_fields);
     type->aggregate.num_fields = num_fields; 
}

void complete_type_union(Type *type, TypeField *fields, size_t num_fields){
     assert(type->kind == TYPE_COMPLETING);
     type->kind = TYPE_UNION;
     type->size = 0;
     for ( size_t i = 0; i < num_fields; i++ ){
          //   TODO : Alignment
          type->size = MAX(type->size, fields[i].type->size);
     }
     type->aggregate.fields = arena_dup(&resolve_arena,fields,sizeof(*fields)*num_fields);
     type->aggregate.num_fields = num_fields;
}
void complete_type(Type *type){
     if ( type->kind == TYPE_COMPLETING ){
          fatal("Mad cycles detected!!\n");
     } else if ( type->kind != TYPE_INCOMPLETE ){
          return;
     }
     assert( type->entity->decl->kind == DECL_STRUCT || type->entity->decl->kind == DECL_UNION );
     aggregate_item *items = type->entity->decl->aggregate_decl.aggregate_items;
     size_t num_items = type->entity->decl->aggregate_decl.num_aggregate_items;
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

     if ( type->entity->decl->kind == DECL_STRUCT ){
          complete_type_struct(type,fields,buff_len(fields));
     } else {
          assert(type->entity->decl->kind = DECL_UNION );
          complete_type_struct(type,fields,buff_len(fields));
     }
     
     buff_push(ordered_entities,type->entity);
}

int64_t resolve_int_const_expr(Expr *expr){
     ResolvedExpr new = resolve_expr(expr);
     if ( !new.is_const ){
          fatal("Expected constant expression");
     }
     return new.int_val;
}

Type *resolve_typespec(TypeSpec *typespec){
     switch ( typespec->kind ){

          case TYPESPEC_NAME:{
               Entity *entity = resolve_name(typespec->name); 
               if ( entity->kind != ENTITY_TYPE ){
                    fatal("%s is not a type!!\n",typespec->name);
                    return NULL;
               }
               return entity->type;
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
          case TYPESPEC_ARRAY:
               return type_array(resolve_typespec(typespec->array.base_type), resolve_int_const_expr(typespec->array.size));
	          break;
          case TYPESPEC_POINTER:
               return type_ptr(resolve_typespec(typespec->ptr.base_type));
               break;
          default: 
               assert(0);
               break;
     }
}

Type *resolve_func_decl(Decl *decl){
     assert(decl->kind == DECL_FUNC);
     /*
          struct {
               Type **param_list;
               Type *ret_type;
               size_t num_params;
          } func;
          */
 //    Type *type_func(Type **param_list,size_t num_params, Type *ret_type){
     Type **param_list = NULL;
     Type *tmp = NULL;     
     for ( size_t i = 0; i < decl->func_decl.num_params; i++ ){
          tmp = resolve_typespec(decl->func_decl.param_list[i].type);
          complete_type(tmp);
          buff_push(param_list,tmp);
     }
     tmp = resolve_typespec(decl->func_decl.ret_type);
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
          fatal("Initializer for a const decl is not a constant expression!\n");
     }
     return x.type;
}


Type *resolve_var_decl(Decl *decl){
     assert(decl->kind == DECL_VAR );
     Type *type = NULL;
     if  ( decl->var_decl.type ){
          type = resolve_typespec(decl->var_decl.type);
     }

     if ( decl->var_decl.expr ){
          ResolvedExpr result = resolve_expr_expected(decl->var_decl.expr,type);
          if ( type && result.type != type ){
               fatal("Expression type dosen't match the variable type!\n");
          }
          type = result.type;
     }
     assert(type!=NULL);
     complete_type(type);
     return type;
}


Type *resolve_type_decl(Decl *decl){
     assert(decl->kind == DECL_TYPEDEF);
     return resolve_typespec(decl->typedef_decl.type);
}

void *resolve_entity(Entity *entity){

     /*
      * Structs and union are treated as incomplete types so they are taken as resolved types.
      * because we don't need definition of structs when using as pointers.
      * Their types are completed when the details of struct are need.
      * So the only unresolved entity type is the typedef
      */
     if ( entity->state == ENTITY_RESOLVED ){
          return entity;
     } else if ( entity->state == ENTITY_RESOLVING ){
          fatal("Cyclic dependency of varaibles!!\n");
     }
     assert(entity->state == ENTITY_UNRESOLVED);
     entity->state = ENTITY_RESOLVING;
     
     switch ( entity->kind ){
         case ENTITY_CONST:
              entity->type = resolve_const_decl(entity->decl,&entity->val);
              break;
         case ENTITY_VAR:
              entity->type = resolve_var_decl(entity->decl);
              break;
         case ENTITY_TYPE:
              entity->type = resolve_type_decl(entity->decl);
              break;
         case ENTITY_FUNC:
              entity->type = resolve_func_decl(entity->decl);
              break;
         default:
               assert(0);
               break;
     }
     entity->state = ENTITY_RESOLVED;
     buff_push(ordered_entities,entity);
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
     
     Entity *new = entity_get(name);
     resolve_entity(new);
     if ( new->kind == ENTITY_VAR ){
          return resolve_lvalue(new->type);
     } else if ( new->kind == ENTITY_CONST ){
          return resolve_const_int(new->val);
     } else if ( new->kind == ENTITY_FUNC){
          return resolve_rvalue(new->type);
     }else {
          fatal("name should be a var or a constant\n");
     }
     return resolve_null;
}

ResolvedExpr resolve_expr_field(Expr *expr){

     ResolvedExpr left = resolve_expr(expr->field_expr.operand);
     Type *type  = left.type;
     complete_type(type);
     if ( type->kind != TYPE_STRUCT &&  type->kind != TYPE_UNION ){
          fatal("field access must be on a struct or a pointer\n");
     }
          
     for ( size_t i = 0; i < type->aggregate.num_fields; i++ ){
          if ( expr->field_expr.field_name == type->aggregate.fields[i].name ){
             return  left.is_lvalue?resolve_lvalue(type->aggregate.fields[i].type):resolve_rvalue(type->aggregate.fields[i].type);
          }
     }
     fatal("Invalid member for a struct\n");
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
          default:
               fatal("Unidentified unary op!\n");
               assert(0);
               break;
     }
     return 0;
}
ResolvedExpr resolve_expr_unary(Expr *expr){
     ResolvedExpr new= resolve_expr(expr->unary_expr.operand);
     Type *type = new.type;
     if ( new.is_const ){
          return resolve_const_int(eval_expr_unary(new.int_val,expr->unary_expr.op));
     }
     return resolve_rvalue(type);
}

ResolvedExpr resolve_expr_binary(Expr *expr){
     ResolvedExpr left = resolve_expr(expr->binary_expr.left);
     ResolvedExpr right = resolve_expr(expr->binary_expr.right);

     if ( left.type != right.type ){
          fatal("Type of values do not match\n"); // TODO
     }
     if ( left.is_const && right.is_const ){
          switch ( expr->binary_expr.op ){
               case TOKEN_ADD:
                    return resolve_const_int(left.int_val+right.int_val);
                    break;
               default:
                    assert(0);
                    break;
          }
     } else {
          return resolve_rvalue(left.type);
     }
}
ResolvedExpr resolve_expr_compound(Expr *expr, Type *expected_type){
    /* 
     TypeSpec *type;
     size_t num_args;
     BUF(Expr **args); // Buffer
     */
   //  assert(expr->compound_expr.type);
     Type *comp_type = NULL;
     if ( !expected_type && !expr->compound_expr.type ){
          fatal("Compound Expr with no designated type!\n");
          assert(0);
     } else if ( expr->compound_expr.type ) {
          comp_type = resolve_typespec(expr->compound_expr.type);
          if ( comp_type  != expected_type ){
               fatal("Expression type does not match the designated type!\n");
               assert(0);
          }
     } 
     
     Type *type = expected_type? expected_type : resolve_typespec(expr->compound_expr.type);
     complete_type(type);
     
     //assert( type->kind == TYPE_STRUCT ); 
     if ( type->kind != TYPE_STRUCT && type->kind != TYPE_UNION && type->kind != TYPE_ARRAY ){
          fatal("Attempt to use compound expression on a non-compound type\n");
     }
     if ( type->kind == TYPE_STRUCT || type->kind == TYPE_UNION){
          if ( expr->compound_expr.num_args > type->aggregate.num_fields ){
               fatal("Compound expr has more members than the type\n");
          }
          ResolvedExpr new = {};
          for ( size_t i = 0; i < expr->compound_expr.num_args; i++ ){
               new = resolve_expr_expected(expr->compound_expr.args[i],type);
               if ( new.type != type->aggregate.fields[i].type ){
                   fatal("Type mismatch for compound literal!\n"); 
               }
          }
     } else {
          assert(type->kind == TYPE_ARRAY );
          if ( expr->compound_expr.num_args > type->array.size ){
               fatal("Array expression has more members than the array length\n");
          }
          ResolvedExpr new = {};
          for ( size_t i = 0; i < expr->compound_expr.num_args; i++ ){
               new = resolve_expr_expected(expr->compound_expr.args[i],type);
               if ( new.type != type->array.base_type ){
                    fatal("Type mismatch with array type\n");
               }
          }
     } 
     return resolve_rvalue(type);
}


ResolvedExpr resolve_expr_call(Expr *expr,Type *expected_type){
     assert(expr->kind == EXPR_CALL);
     ResolvedExpr new = resolve_expr(expr->func_call_expr.operand);
     Type *type = new.type;
     if ( type->kind != TYPE_FUNC ){
          fatal("Undeclared function being called!\n");
          assert(0);
     } 
     ResolvedExpr result = {};
     if ( type->func.num_params != expr->func_call_expr.num_args ){
          fatal("Number of arguments of function call dosen't match with declaration!\n");
          assert(0);
     } 
     for ( size_t i = 0; i < type->func.num_params; i++ ){
          result = resolve_expr_expected(expr->func_call_expr.args[i],type->func.param_list[i]);
          if ( result.type != type->func.param_list[i] ){
               fatal("Function Argument type mismtach \n");
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
          fatal("The value of condition expr is not an integer!\n");
          assert(0);
     }
     if ( then_expr.type != else_expr.type ){
          fatal("Type mismatch for condition expressions of ternary expression!\n");
          assert(0);
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
          if ( !new.is_lvalue || (new.type->kind != TYPE_PTR && new.type->kind != TYPE_INT) ){
               fatal("Invalid expression to cast to pointer type.\n");
          }
     } else if ( cast_type->kind == TYPE_INT ){
          if ( new.type->kind != TYPE_PTR && new.type->kind != TYPE_INT ){
               fatal("Invalid expression to cast to integer type.\n");
          }
     } else {
          fatal("Invalid cast type!\n");
     }
     return resolve_rvalue(cast_type);
}

ResolvedExpr resolve_expr_expected(Expr *expr,Type *expected_type){
     switch (expr->kind){
          case EXPR_INT:
               return resolve_const_int(expr->int_val);
               break;
          case EXPR_FLOAT:
               return resolve_const_float(expr->float_val);
               break;
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
               Type *type = resolve_typespec(expr->sizeof_type);
               complete_type(type);
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

void entity_add_type(Type *type){
     Entity *new = NULL;
     if ( type->kind == TYPE_INT ){
         new = new_entity(ENTITY_TYPE,str_intern("int"),NULL); 
     } else if ( type->kind == TYPE_FLOAT ){
         new = new_entity(ENTITY_TYPE,str_intern("float"),NULL); 
     }
     new->state = ENTITY_RESOLVED;
     new->type = type;
     buff_push(entity_list,new);
}


void resolve_test(void){
     char *list[] = { 
          "const x = ~1;",
//          "var x = cast(int)1;",
//          "var n:int = 1;",
 //         "const x = 1 ? n : 3;",
//          "func addV(x:Vector,y:Vector):Vector { return {p.x+q.x,p.y+q.y}; }",
//         "func add(x:int,y:int):int{ return x+y;}",
//          "var z:int = add(1,2);",
//          "var c:int[3] = {1,2,3};",
//          "struct Vector { x,y:int; };",
//          "var e:int* = &p",
//          "var p:int = 42",
//          "var q:Vector = {12,999}",
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
//          "var q = &i;"
     };
     Decl *tmp_decl;
     for( char **it = list; it != list + sizeof(list)/sizeof(char *); it++ ){
          init_stream(*it);
          tmp_decl = parse_decl();
          entity_add_decl(tmp_decl);
     }
     entity_add_type(type_int);
     entity_add_type(type_float);
     for ( Entity **it = entity_list; it != buff_end(entity_list); it++ ){
          resolve_entity(*it);
          complete_type((*it)->type);
     }

     for ( Entity **it = ordered_entities; it != buff_end(ordered_entities); it++ ){
          print_decl( (*it)->decl );
          printf("\n");
     }
}
