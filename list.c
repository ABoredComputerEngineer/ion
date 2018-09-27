typedef struct SymNode SymNode;
struct SymNode {
     SymNode *next;
     Sym *sym;
} ;

extern Arena resolve_arena;

SymNode *newSymNode(Sym *sym){
     SymNode *new = arena_alloc(&resolve_arena,sizeof(SymNode));
     new->next= NULL;
     new->sym = sym;
     return new;
}

SymNode *addSymNode(SymNode *head, SymNode *new){
     if ( head == NULL ){
          head = new;
         return new;
     }
     SymNode *it = head;
     for ( ; it->next != NULL; it = it->next );
     it->next = new;
     return head;
}

bool in_list(SymNode *head,Sym *sym){
     for ( SymNode *it = head; it != NULL ; it = it->next ){
          if ( it->sym == sym ){
               return true;
          }
     }
     return false;
}
/*
SymNode *delete_SymNode(SymNode *head,  Sym *sym){
     if ( head == NULL )
          return head;
     if ( head->sym == sym )
         SymNode *new_head = head->next;
         free(head);
         return new_head;
     for ( SymNode *it = head; it->next != NULL ; it = it->next ){
          if ( it->sym == sym ){
               SymNode *tmp = it->next->next;
               free(it->next);
               it->next = tmp;
               return head;
          }
     }
     
     return head;
}
*/
