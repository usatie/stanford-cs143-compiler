

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

bool ClassTable::has_cyclic_inheritance(Class_ orig, Class_ curr) {
  // Check cycles already detected
  if (class_table.probe(curr->get_name()) != NULL) {
    return true;
  }
  // If no parent -> no cycle
  auto parent = class_table.lookup(curr->get_parent_sym());
  if (parent == NULL) {
    return false; // parent is a basic class
  }
  // Check if the parent class is the original class (i.e. cycle detected)
  if (orig == parent) {
    class_table.addid(curr->get_name(), curr);
    semant_error(curr) << "Class " << curr->get_name() << ", or an ancestor of "
                       << curr->get_name()
                       << ", is involved in an inheritance cycle." << std::endl;
    return true;
  }
  // Check if the ancestor classes have cyclic inheritance
  if (has_cyclic_inheritance(orig, parent)) {
    class_table.addid(curr->get_name(), curr);
    semant_error(curr) << "Class " << curr->get_name() << ", or an ancestor of "
                       << curr->get_name()
                       << ", is involved in an inheritance cycle." << std::endl;
    return true;
  } else {
    // TODO: Maybe we can store the curr to non_cyclic_classes
    return false;
  }
}

ClassTable::ClassTable(Classes classes)
    : semant_errors(0), error_stream(cerr), visiting(NULL) {

  /* Fill this in */

  /* install basic classes */
  install_basic_classes();

  /* Install user-defined classes */
  install_user_defined_classes(classes);

  /* Check Illegal Undefined/Basic class inheritance */
  if (semant_debug) {
    std::cout << "Checking undefined/basic class inheritance..." << std::endl;
  }
  // Use a new scope to manage the already checked classes
  class_table.enterscope();
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    if (class_table.probe(name) != NULL) {
      continue;
    }
    class_table.addid(name, classes->nth(i));
    Symbol parent_sym = classes->nth(i)->get_parent_sym();
    // The parent is illegal basic class
    if (parent_sym == Bool || parent_sym == Int || parent_sym == Str) {
      semant_error(classes->nth(i))
          << "Class " << name << " cannot inherit class " << parent_sym << "."
          << std::endl;
      continue;
    }
    // The parent is undefined class
    auto parent = class_table.lookup(parent_sym);
    if (parent == NULL) {
      semant_error(classes->nth(i))
          << "Class " << name << " inherits from an undefined class "
          << parent_sym << "." << std::endl;
      continue;
    }
    classes->nth(i)->set_parent(parent);
  }
  class_table.exitscope();

  /* Check Illegal Cyclic inheritance */
  if (semant_debug) {
    std::cout << "Checking cyclic inheritance..." << std::endl;
  }
  // Use a new scope to manage the already checked classes
  class_table.enterscope();
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    auto curr = classes->nth(i);
    has_cyclic_inheritance(curr, curr);
  }
  class_table.exitscope();
}

void ClassTable::install_user_defined_classes(Classes classes) {
  class_table.enterscope();
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    Symbol name = classes->nth(i)->get_name();
    if (class_table.probe(name) != NULL) {
      semant_error(classes->nth(i))
          << "Class " << name << " was previously defined." << std::endl;
    } else if (class_table.lookup(name) != NULL) {
      semant_error(classes->nth(i))
          << "Redefinition of basic class " << name << std::endl;
    } else {
      class_table.addid(name, classes->nth(i));
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

  /* Set parent classes */
  IO_class->set_parent(Object_class);
  Int_class->set_parent(Object_class);
  Bool_class->set_parent(Object_class);
  Str_class->set_parent(Object_class);

  /* Add basic classes to the class table */
  class_table.enterscope();
  class_table.addid(Object, Object_class);
  class_table.addid(IO, IO_class);
  class_table.addid(Int, Int_class);
  class_table.addid(Bool, Bool_class);
  class_table.addid(Str, Str_class);
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

ostream &ClassTable::semant_error(tree_node *t) {
  return semant_error(visiting->get_filename(), t);
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
static bool conforms_to(Class_ A, Class_ B, InternalClassTable &class_table) {
  if (A == NULL) {
    return false;
  }
  // A ≤ A for all types A
  if (A == B) {
    return true;
  }
  // if C inherits from P, then C ≤ P
  if (A->get_parent_sym() == B->get_name()) {
    return true;
  }
  // if A ≤ C and C ≤ P then A ≤ P
  auto parent = class_table.lookup(A->get_parent_sym());
  return conforms_to(parent, B, class_table);
}

bool method_class::is_method() { return true; }
bool attr_class::is_method() { return false; }

void class__class::semant(ClassTableP classtable) {
  if (semant_debug) {
    std::cout << "class__class::semant" << std::endl;
  }
  if (classtable->is_visited(this)) {
    return;
  }
  // TODO: install symbols from anscestor classes
  // Install symbols from this class
  classtable->method_table.enterscope();
  classtable->symtab.enterscope();
  classtable->symtab.addid(self, this);
  // TODO: Semantic analysis for this class
  // First, check all attributes (and install them to the symbol table)
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    if (feature->is_method()) {
      continue;
    }
    feature->semant(classtable);
  }
  // Second, check all methods (and add them to the symbol table inside)
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    if (!feature->is_method()) {
      continue;
    }
    feature->semant(classtable);
  }
  classtable->symtab.exitscope();
  classtable->method_table.exitscope();

  // Mark this class as visited
  classtable->mark_visited(this);
}

void method_class::semant(ClassTableP classtable) {
  if (semant_debug) {
    std::cout << "method_class::semant" << std::endl;
  }
  if (classtable->method_table.lookup(name) != NULL) {
    classtable->semant_error(this)
        << "Method " << name << " is multiply defined." << std::endl;
  } else {
    classtable->method_table.addid(name, this);
  }
  if (classtable->lookup_class(return_type) == NULL) {
    classtable->semant_error(this) << "Undefined return type " << return_type
                                   << " in method " << name << "." << std::endl;
  }
  // Install the names of the formals in the symbol table
  classtable->symtab.enterscope();
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    auto formal = formals->nth(i);
    formal->semant(classtable);
  }
  // TODO: Check if expr type matches return_type
  expr->semant(classtable);
  classtable->symtab.exitscope();
}

void attr_class::semant(ClassTableP classtable) {
  if (semant_debug) {
    std::cout << "attr_class::semant" << std::endl;
  }
  init->semant(classtable);
  if (classtable->symtab.lookup(name) != NULL) {
    classtable->semant_error(this)
        << "Attribute " << name << " is multiply defined in class."
        << std::endl;
  } else {
    classtable->symtab.addid(name, this);
  }
  if (classtable->lookup_class(type_decl) == NULL) {
    classtable->semant_error(this) << "Class " << type_decl << " of attribute "
                                   << name << " is undefined." << std::endl;
  }
}

void formal_class::semant(ClassTableP classtable) {
  if (semant_debug) {
    std::cout << "formal_class::semant" << std::endl;
  }
  if (classtable->symtab.probe(name) != NULL) {
    classtable->semant_error(this)
        << "Formal parameter " << name << " is multiply defined." << std::endl;
  } else {
    classtable->symtab.addid(name, this);
  }
  if (classtable->lookup_class(type_decl) == NULL) {
    classtable->semant_error(this)
        << "Class " << type_decl << " of formal parameter " << name
        << " is undefined." << std::endl;
  }
}

void no_expr_class::semant(ClassTableP classtable) {}
void isvoid_class::semant(ClassTableP classtable) { e1->semant(classtable); }
void new__class::semant(ClassTableP classtable) {
  if (classtable->lookup_class(type_name) == NULL) {
    classtable->semant_error(this)
        << "'new' used with undefined class " << type_name << "." << std::endl;
  }
}
void string_const_class::semant(ClassTableP classtable) {}
void bool_const_class::semant(ClassTableP classtable) {}
void int_const_class::semant(ClassTableP classtable) {}
void comp_class::semant(ClassTableP classtable) { e1->semant(classtable); }
void leq_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void eq_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void lt_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void neg_class::semant(ClassTableP classtable) { e1->semant(classtable); }
void divide_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void mul_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void sub_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void plus_class::semant(ClassTableP classtable) {
  e1->semant(classtable);
  e2->semant(classtable);
}
void let_class::semant(ClassTableP classtable) { /* TODO: Implement */
  init->semant(classtable);
  if (classtable->lookup_class(type_decl) == NULL) {
    classtable->semant_error(this)
        << "Class " << type_decl << " of let-bound identifier " << identifier
        << " is undefined." << std::endl;
  }
  // TODO: Check if the type_decl and type(init) matches
  classtable->symtab.enterscope();
  classtable->symtab.addid(identifier, this);
  body->semant(classtable);
  classtable->symtab.exitscope();
}
void block_class::semant(ClassTableP classtable) {
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    body->nth(i)->semant(classtable);
  }
}
void typcase_class::semant(ClassTableP classtable) { /* TODO: Implement */
}
void loop_class::semant(ClassTableP classtable) {
  pred->semant(classtable);
  body->semant(classtable);
}
void cond_class::semant(ClassTableP classtable) {
  pred->semant(classtable);
  then_exp->semant(classtable);
  else_exp->semant(classtable);
}
void dispatch_class::semant(ClassTableP classtable) {
  expr->semant(classtable);
  // TODO: Check if name is a method of the class of the expr
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    actual->nth(i)->semant(classtable);
  }
}
void static_dispatch_class::semant(
    ClassTableP classtable) { /* TODO: Implement */
  expr->semant(classtable);
  // TODO: Check if typename is a parent class of the class of the expr
  // TODO: Check if name is a method of the parent class
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    actual->nth(i)->semant(classtable);
  }
}
void assign_class::semant(ClassTableP classtable) {
  if (classtable->symtab.lookup(name) == NULL) {
    classtable->semant_error(this)
        << "Assignment to undeclared variable " << name << "." << std::endl;
  }
  expr->semant(classtable);
}

void object_class::semant(ClassTableP classtable) {
  if (semant_debug) {
    std::cout << "object_class::semant" << std::endl;
  }
  if (classtable->symtab.lookup(name) == NULL) {
    classtable->semant_error(this)
        << "Undeclared identifier " << name << "." << std::endl;
  }
}

void ClassTable::check_name_and_scope(Classes classes) {
  if (semant_debug) {
    std::cout << "Checking naming and scoping..." << std::endl;
  }
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    visiting = classes->nth(i);
    class_table.enterscope();
    visiting->semant(this);
    class_table.exitscope();
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
  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
  /* TODO: Check Naming and Scoping */
  classtable->check_name_and_scope(classes);
  /* TODO: Type Checking */
  classtable->check_type();

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
}

Class_ ClassTable::lookup_class(Symbol name) {
  return class_table.lookup(name);
}

bool ClassTable::is_visited(Class_ c) {
  return class_table.probe(c->get_name()) != NULL;
}

void ClassTable::mark_visited(Class_ c) { class_table.addid(c->get_name(), c); }
