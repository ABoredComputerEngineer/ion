
typedef struct Decl Decl;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef enum TypeSpecKind TypeSpecKind;
typedef struct TypeSpec TypeSpec;

typedef struct StmtBlock {
     size_t num_stmts;
     BUF(Stmt **stmts); // Buffer to hold a list of statements
} StmtBlock;
enum TypeSpecKind {
     TYPESPEC_NAME,
     TYPESPEC_PAREN,
     TYPESPEC_FUNC,
     TYPESPEC_ARRAY,
     TYPESPEC_POINTER
};

typedef struct func_typespec{
     struct{
          BUF( TypeSpec **args;) // Buffer to hold the data type of arguments given to a function ) )
          size_t num_args;
     };
     TypeSpec *ret_type; // The return type of the function
} func_typespec;

struct TypeSpec{
     TypeSpecKind kind;
     struct {
          const char *name; // Type name
          // array  and pointer
          struct {
               TypeSpec *base_type; // Base type of an array
               Expr *size; // Index of the array
          };
          func_typespec func_decl;
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
     STMT_INIT,
     STMT_ASSIGN,
     STMT_EXPR,
     STMT_FOR,
     STMT_WHILE,
     STMT_DO_WHILE,
     STMT_IF,
     STMT_SWITCH,
     STMT_RETURN,
     STMT_BREAK,
     STMT_CONTINUE,
     STMT_BLOCK
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
     BUF(func_param *param_list); // Buffer for storing function parameter list
     size_t num_params;
     TypeSpec *ret_type;  // return type of the function
     StmtBlock block;
} func_def;

typedef struct enum_def{
     size_t num_enum_items;
     enum_item *enum_items; // Buffer for storing items of enum declaration,e.g. ( name : expr )
} enum_def;

typedef struct aggregate_def{
     size_t num_aggregate_items;
     aggregate_item *aggregate_items; // Buffer to store the name value pairs for structures and unions``
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



typedef struct Elseif {
     Expr *cond;
     StmtBlock elseif_block;
} Elseif;

typedef struct stmt_if_def {
     Expr *cond;  // the condition for if statement
     StmtBlock if_block;
     size_t num_elseifs;
     Elseif *elseifs; // list of pointers to else_if blocks
     StmtBlock else_block;
} stmt_if_def;

typedef struct stmt_assign_def { // Colon assign
     const char *name;
     Expr *rhs;
} stmt_assign_def;

typedef struct stmt_op_assign_def {
     Expr *lhs;
     TokenKind op;
//     const char *name;
     Expr *rhs;
}  stmt_op_assign_def;

typedef struct Case {
     // supports list of exprs
     Expr **expr_list; // buffer to hold a list of expressions
     size_t num_expr;
     bool isdefault;
     StmtBlock case_block;
} Case;

typedef struct stmt_switch_def{
     Expr *expr; 
     size_t num_cases;
    BUF( Case *cases;) // buffer to hold all the case statements;
} stmt_switch_def;

typedef struct stmt_for_def{
     Stmt *stmt_init;
     Stmt *stmt_update;
     Expr *expr_cond;
     StmtBlock block; // Statements block for for, while... statements, if block for if statement.
} stmt_for_def;

typedef struct stmt_return_def {
     Expr *return_expr;
} stmt_return_def;

typedef struct stmt_while_def {
     Expr *expr_cond;
     StmtBlock block; // Statements block for for, while... statements, if block for if statement.
} stmt_while_def;

struct Stmt{
     StmtKind kind;
//     Expr *expr; //  for statements like for, while ,return, switch do while etc, cond expression for if .

     union {
          stmt_return_def return_stmt;
          stmt_while_def while_stmt;
                    
          stmt_if_def if_stmt;
          stmt_for_def for_stmt;
          stmt_switch_def switch_stmt;
          stmt_assign_def assign_stmt;
          Expr *expr_stmt;
          StmtBlock block;
     };

};

