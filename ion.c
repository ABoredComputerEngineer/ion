extern bool use_gen_buff;
extern int indent;

void init_compiler(void){
     sym_add_type(str_intern("int"),type_int);
     sym_add_type(str_intern("float"),type_float);
     sym_add_type(str_intern( "char" ),type_char);
     sym_add_type(str_intern( "void" ),type_void);
     use_gen_buff = true;
     indent = 0;
     init_intern_keyword();
}

bool ion_compile_file(const char *path){
     init_compiler();
     const char *stream = open_ion_file(path);
     init_stream((char *)stream);
     DeclList *decls = parse_decls();
     for ( size_t i = 0 ; i < decls->num_decls ; i++ ){
          sym_add_decl( decls->decl_list[i]);
     }
     finalize_syms();
     gen_forward_decls();
     for( Sym **it = ordered_syms; it != buff_end(ordered_syms); it++ ){
          gen_code(*it);
     }

    export_to_c(path,gen_buff,strlen(gen_buff));
    return true;
}

void ion_test(void){
     const char *path = "./new.ion";
     ion_compile_file(path);
}

bool ion_main(int argc, char **argv){
     if ( argc == 1 ){
          printf("Ion Complier. Version -1!\n");
     }
     assert(argc>1);
     size_t len = strlen(argv[1]);
     char *path = malloc(len*sizeof(char) + 1);
//     snprintf(path,len*sizeof(char)+1,"%s",argv[1]);
     strcpy( path, argv[1] ); 
     if ( ion_compile_file(path) ){
          printf("Compilation Succesfull!\n");
     } else {
          printf("Compliation unsuccesful!\n");
     }
     return true;
}

