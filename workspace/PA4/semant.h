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
  bool has_cyclic_inheritance(Class_ orig, Class_ curr);
  ostream &error_stream;

public:
  InternalClassTable class_table;
  InternalClassTable branch_table;
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream &semant_error();
  ostream &semant_error(Class_ c);
  ostream &semant_error(Symbol filename, tree_node *t);

  void semant_cyclic_inheritance(Classes classes);
  void semant_name_scope(Classes classes);
  method_class *lookup_method(Class_ cls, Symbol name);
  void semant_main();
};

class TypeEnvironment {
private:
  typedef SymbolTable<Symbol, Entry> ObjectEnvironment;
  typedef SymbolTable<Symbol, method_class> MethodEnvironment;
  typedef Class_ CurrentClass;

public:
  TypeEnvironment(ClassTableP ct);
  ~TypeEnvironment();
  ClassTableP classtable;
  ObjectEnvironment o;
  MethodEnvironment m;
  CurrentClass c;
  ostream &semant_error(tree_node *t);
  int errors() { return classtable->errors(); }
  Class_ lookup_class(Symbol name) {
    return classtable->class_table.lookup(name);
  }
};

#endif
