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
  // traverse with function pointer for pre and post
  typedef void (ClassTable::*traverse_func)(Class_ c);
  void traverse_classes(Classes classes, int i, traverse_func pre,
                        traverse_func post);
  void install_class(Class_ c);
  void validate_inheritance(Class_ c);
  ostream &error_stream;
  InternalClassTable class_table;
  Class_ visiting;

public:
  InternalClassTable object_table;
  SymTab method_table;
  InternalClassTable branch_table;
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream &semant_error();
  ostream &semant_error(Class_ c);
  ostream &semant_error(tree_node *t);
  ostream &semant_error(Symbol filename, tree_node *t);

  void semant_cyclic_inheritance(Classes classes);
  void semant_name_scope(Classes classes);
  void check_type();
  bool has_cyclic_inheritance(Class_ orig, Class_ curr);
  Class_ lookup_class(Symbol name);
  bool conforms_to(Symbol A, Symbol B);
  Symbol join_type(Symbol s1, Symbol s2);
  method_class *lookup_method(Class_ cls, Symbol name);
};

#endif
