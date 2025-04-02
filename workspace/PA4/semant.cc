

#include "semant.h"
#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol arg, arg2, Bool, concat, cool_abort, copy, Int, in_int, in_string,
    IO, length, Main, main_meth, No_class, No_type, Object, out_int, out_string,
    prim_slot, self, SELF_TYPE, Str, str_field, substr, type_name, val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void) {
  arg = idtable.add_string("arg");
  arg2 = idtable.add_string("arg2");
  Bool = idtable.add_string("Bool");
  concat = idtable.add_string("concat");
  cool_abort = idtable.add_string("abort");
  copy = idtable.add_string("copy");
  Int = idtable.add_string("Int");
  in_int = idtable.add_string("in_int");
  in_string = idtable.add_string("in_string");
  IO = idtable.add_string("IO");
  length = idtable.add_string("length");
  Main = idtable.add_string("Main");
  main_meth = idtable.add_string("main");
  //   _no_class is a symbol that can't be the name of any
  //   user-defined class.
  No_class = idtable.add_string("_no_class");
  No_type = idtable.add_string("_no_type");
  Object = idtable.add_string("Object");
  out_int = idtable.add_string("out_int");
  out_string = idtable.add_string("out_string");
  prim_slot = idtable.add_string("_prim_slot");
  self = idtable.add_string("self");
  SELF_TYPE = idtable.add_string("SELF_TYPE");
  Str = idtable.add_string("String");
  str_field = idtable.add_string("_str_field");
  substr = idtable.add_string("substr");
  type_name = idtable.add_string("type_name");
  val = idtable.add_string("_val");
}

bool ClassTable::has_cyclic_inheritance(
    Class_ orig, Class_ curr, SymbolTable<char *, Class__class> *class_table,
    SymbolTable<char *, Class__class> *cycle_table) {
  // Check cycles already detected
  if (cycle_table->lookup(curr->get_name()->get_string()) != NULL) {
    return true;
  }
  // Check if the class is the original class (if not, it should be basic class
  // -> no cycle)
  if (class_table->lookup(curr->get_name()->get_string()) == NULL) {
    return false;
  }
  auto parent = class_table->lookup(curr->get_parent()->get_string());
  if (parent == NULL) {
    // TODO: Maybe we can store the curr to non_cyclic_classes
    return false; // parent is a basic class
  }
  // Check if the parent class is the original class (i.e. cycle detected)
  if (orig == parent) {
    cycle_table->addid(curr->get_name()->get_string(), curr);
    semant_error(curr) << "Class " << curr->get_name() << ", or an ancestor of "
                       << curr->get_name()
                       << ", is involved in an inheritance cycle." << std::endl;
    return true;
  }
  // Check if the ancestor classes have cyclic inheritance
  if (has_cyclic_inheritance(orig, parent, class_table, cycle_table)) {
    cycle_table->addid(curr->get_name()->get_string(), curr);
    semant_error(curr) << "Class " << curr->get_name() << ", or an ancestor of "
                       << curr->get_name()
                       << ", is involved in an inheritance cycle." << std::endl;
    return true;
  } else {
    // TODO: Maybe we can store the curr to non_cyclic_classes
    return false;
  }
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {

  /* Fill this in */
  class_table = new SymbolTable<char *, Class__class>();
  class_table->enterscope();

  /* install basic classes */
  install_basic_classes();

  /* Check Basic class inheritance */
  if (semant_debug) {
    std::cout << "Checking basic class inheritance..." << std::endl;
  }
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    if (basic_class_table->lookup(name->get_string()) != NULL) {
      semant_error(classes->nth(i))
          << "Redefinition of basic class " << name << std::endl;
    }
  }

  /* Install user-defined classes */
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    if (class_table->lookup(name->get_string()) != NULL) {
      semant_error(classes->nth(i))
          << "Class " << name << " was previously defined." << std::endl;
    }
    class_table->addid(name->get_string(), classes->nth(i));
  }

  /* Check Cyclic inheritance */
  if (semant_debug) {
    std::cout << "Checking cyclic inheritance..." << std::endl;
  }
  SymbolTable<char *, Class__class> *cycle_table =
      new SymbolTable<char *, Class__class>();
  cycle_table->enterscope();
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    auto curr = classes->nth(i);
    has_cyclic_inheritance(curr, curr, class_table, cycle_table);
  }
  delete cycle_table;

  /* Check Undefined class inheritance */
  if (semant_debug) {
    std::cout << "Checking undefined class inheritance..." << std::endl;
  }
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    Symbol parent = classes->nth(i)->get_parent();
    if (basic_class_table->lookup(parent->get_string()) == NULL &&
        class_table->lookup(parent->get_string()) == NULL) {
      semant_error(classes->nth(i))
          << "Class " << name << " inherits from an undefined class " << parent
          << std::endl;
    }
  }
}

void ClassTable::install_basic_classes() {

  // The tree package uses these globals to annotate the classes built below.
  // curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

  // The following demonstrates how to create dummy parse trees to
  // refer to basic Cool classes.  There's no need for method
  // bodies -- these are already built into the runtime system.

  // IMPORTANT: The results of the following expressions are
  // stored in local variables.  You will want to do something
  // with those variables at the end of this method to make this
  // code meaningful.

  //
  // The Object class has no parent class. Its methods are
  //        abort() : Object    aborts the program
  //        type_name() : Str   returns a string representation of class name
  //        copy() : SELF_TYPE  returns a copy of the object
  //
  // There is no need for method bodies in the basic classes---these
  // are already built in to the runtime system.

  Class_ Object_class = class_(
      Object, No_class,
      append_Features(
          append_Features(single_Features(method(cool_abort, nil_Formals(),
                                                 Object, no_expr())),
                          single_Features(method(type_name, nil_Formals(), Str,
                                                 no_expr()))),
          single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
      filename);

  //
  // The IO class inherits from Object. Its methods are
  //        out_string(Str) : SELF_TYPE       writes a string to the output
  //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
  //        in_string() : Str                 reads a string from the input
  //        in_int() : Int                      "   an int     "  "     "
  //
  Class_ IO_class = class_(
      IO, Object,
      append_Features(
          append_Features(
              append_Features(single_Features(method(
                                  out_string, single_Formals(formal(arg, Str)),
                                  SELF_TYPE, no_expr())),
                              single_Features(method(
                                  out_int, single_Formals(formal(arg, Int)),
                                  SELF_TYPE, no_expr()))),
              single_Features(
                  method(in_string, nil_Formals(), Str, no_expr()))),
          single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
      filename);

  //
  // The Int class has no methods and only a single attribute, the
  // "val" for the integer.
  //
  Class_ Int_class = class_(
      Int, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

  //
  // Bool also has only the "val" slot.
  //
  Class_ Bool_class = class_(
      Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

  //
  // The class Str has a number of slots and operations:
  //       val                                  the length of the string
  //       str_field                            the string itself
  //       length() : Int                       returns length of the string
  //       concat(arg: Str) : Str               performs string concatenation
  //       substr(arg: Int, arg2: Int): Str     substring selection
  //
  Class_ Str_class = class_(
      Str, Object,
      append_Features(
          append_Features(
              append_Features(
                  append_Features(
                      single_Features(attr(val, Int, no_expr())),
                      single_Features(attr(str_field, prim_slot, no_expr()))),
                  single_Features(
                      method(length, nil_Formals(), Int, no_expr()))),
              single_Features(method(concat, single_Formals(formal(arg, Str)),
                                     Str, no_expr()))),
          single_Features(
              method(substr,
                     append_Formals(single_Formals(formal(arg, Int)),
                                    single_Formals(formal(arg2, Int))),
                     Str, no_expr()))),
      filename);

  /* Add basic classes to the class table */
  basic_class_table = new SymbolTable<char *, Class__class>();
  basic_class_table->enterscope();
  basic_class_table->addid(Object->get_string(), Object_class);
  basic_class_table->addid(IO->get_string(), IO_class);
  basic_class_table->addid(Int->get_string(), Int_class);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream &ClassTable::semant_error(Class_ c) {
  return semant_error(c->get_filename(), c);
}

ostream &ClassTable::semant_error(Symbol filename, tree_node *t) {
  error_stream << filename << ":" << t->get_line_number() << ": ";
  return semant_error();
}

ostream &ClassTable::semant_error() {
  semant_errors++;
  return error_stream;
}

////////////////////////////////////////////////////////////////////
//
// TODO: check_name_and_scope and check_type
//
///////////////////////////////////////////////////////////////////
static bool conforms_to(Class_ A, Class_ B,
                        SymbolTable<char *, Class__class> *class_table) {
  if (A == NULL) {
    return false;
  }
  // A ≤ A for all types A
  if (A == B) {
    return true;
  }
  // if C inherits from P, then C ≤ P
  if (A->get_parent() == B->get_name()) {
    return true;
  }
  // if A ≤ C and C ≤ P then A ≤ P
  auto parent = class_table->lookup(A->get_parent()->get_string());
  return conforms_to(parent, B, class_table);
}
void ClassTable::check_name_and_scope() {
  if (semant_debug) {
    std::cout << "Checking naming and scoping..." << std::endl;
  }
}

void ClassTable::check_type() {
  if (semant_debug) {
    std::cout << "Checking type system..." << std::endl;
  }
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant() {
  initialize_constants();

  /* ClassTable constructor may do some semantic analysis */
  ClassTable *classtable = new ClassTable(classes);

  /* some semantic analysis code may go here */
  /* TODO: Check Naming and Scoping */
  classtable->check_name_and_scope();
  /* TODO: Type Checking */
  classtable->check_type();

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
}
