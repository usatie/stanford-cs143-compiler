#ifndef SEMANT_H_
#define SEMANT_H_

#include "cool-tree.h"
#include "list.h"
#include "stringtab.h"
#include "symtab.h"
#include <assert.h>
#include <iostream>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;
typedef SymbolTable<Symbol, Class__class> InternalClassTable;
typedef SymbolTable<Symbol, tree_node> SymTab;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  void install_user_defined_classes(Classes classes);
  ostream &error_stream;
  InternalClassTable class_table;
  Class_ visiting;

public:
  SymTab symtab;
  SymTab method_table;
  InternalClassTable branch_table;
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream &semant_error();
  ostream &semant_error(Class_ c);
  ostream &semant_error(tree_node *t);
  ostream &semant_error(Symbol filename, tree_node *t);

  void check_name_and_scope(Classes classes);
  void check_type();
  bool has_cyclic_inheritance(Class_ orig, Class_ curr);
  Class_ lookup_class(Symbol name);
  bool is_visited(Class_ curr);
  void mark_visited(Class_ curr);
};

#endif
