

#include "semant.h"
#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern int semant_debug;
extern char *curr_filename;

// TODO: Think more about the implementation of self to object_table (in order
// to print SELF_TYPE)

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
static Class_ Object_class;
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

  Object_class = class_(
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

ClassTable::ClassTable(Classes classes)
    : semant_errors(0), error_stream(cerr), visiting(NULL) {

  /* Fill this in */

  /* install basic classes */
  install_basic_classes();

  /* Install user-defined classes */
  class_table.enterscope();
  // Preorder : Install classes
  // Postorder : Inheritance check
  traverse_classes(classes, 0, &ClassTable::install_class,
                   &ClassTable::validate_inheritance);
}

/* Check Illegal Undefined/Basic class inheritance */
// 1. Install user-defined classes
// 2. Recursive call to install user-defined class
// 2-2. Redefinition or previously defined class check
// 3. Undefined/Basic class check
void ClassTable::install_class(Class_ c) {
  auto name = c->get_name();
  if (class_table.probe(name) != NULL) {
    semant_error(c) << "Class " << name << " was previously defined."
                    << std::endl;
  } else if (class_table.lookup(name) != NULL) {
    semant_error(c) << "Redefinition of basic class " << name << "."
                    << std::endl;
  } else {
    class_table.addid(name, c);
  }
}

////////////////////////////////////////////////////////////////////
//
// validate_inheritance is a method that checks for illegal
// inheritance of classes. It checks if the class is inheriting from
// a basic class or an undefined class.
//
//    void ClassTable::validate_inheritance(Class_ c)
//
///////////////////////////////////////////////////////////////////
void ClassTable::validate_inheritance(Class_ c) {
  if (class_table.lookup(c->get_name()) != c) {
    return; // This class is not installed (No need to check)
  }
  Symbol parent_sym = c->get_parent_sym();
  auto parent = class_table.lookup(parent_sym);
  // The parent is illegal basic class
  if (parent_sym == Bool || parent_sym == Int || parent_sym == Str ||
      parent_sym == SELF_TYPE) {
    semant_error(c) << "Class " << c->get_name() << " cannot inherit class "
                    << parent_sym << "." << std::endl;
  }
  // The parent is undefined class
  else if (parent == NULL) {
    semant_error(c) << "Class " << c->get_name()
                    << " inherits from an undefined class " << parent_sym << "."
                    << std::endl;
  }
}

////////////////////////////////////////////////////////////////////
//
// traverse_classes is a method that traverses the classes in the
// order they are defined. It can be used to perform a pre-order and
// post-order traversal of the class definition order.
//
//    void ClassTable::traverse_classes(Classes classes, int i, traverse_func
//    pre,
//                                  traverse_func post)
//
///////////////////////////////////////////////////////////////////
void ClassTable::traverse_classes(Classes classes, int i, traverse_func pre,
                                  traverse_func post) {
  if (!classes->more(i)) {
    return;
  }
  auto c = classes->nth(i);
  // Preorder traversal
  if (pre) {
    (this->*pre)(c);
  }

  // traverse
  traverse_classes(classes, classes->next(i), pre, post);

  // Postorder traversal
  if (post) {
    (this->*post)(c);
  }
}

////////////////////////////////////////////////////////////////////
//
// semant_cyclic_inheritance is a method that checks for cyclic
// inheritance in the class hierarchy. It uses a depth-first
// search approach to traverse the class inheritance hierarchy and
// detect cycles.
//
//    void ClassTable::semant_cyclic_inheritance(Classes classes)
//
///////////////////////////////////////////////////////////////////
void ClassTable::semant_cyclic_inheritance(Classes classes) {
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
// Install features
//
///////////////////////////////////////////////////////////////////
void class__class::install_features(ClassTable *classtable) {
  // Install symbols from anscestor classes
  auto parent = classtable->lookup_class(get_parent_sym());
  if (parent != NULL) {
    parent->install_features(classtable);
  }
  // Install symbols from features
  // Methods:
  //   1. Allow overriding ancestor methods
  //   2. however, duplicate methods are not allowed
  // Attributes:
  //   1. Inherited attributes cannot be redefined.
  classtable->method_table.enterscope();
  classtable->object_table.enterscope();
  classtable->object_table.addid(self, this);
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    auto name = feature->get_name();
    auto method = dynamic_cast<method_class *>(feature);
    auto attr = dynamic_cast<attr_class *>(feature);
    if (method != NULL) {
      if (classtable->method_table.probe(name) != NULL) {
        classtable->semant_error(method)
            << "Method " << name << " is multiply defined." << std::endl;
      } else if (classtable->method_table.lookup(name) != NULL) {
        classtable->method_table.addid(name, feature);
      } else {
        classtable->method_table.addid(name, feature);
      }
    } else {
      if (name == self) {
        classtable->semant_error(attr)
            << "'self' cannot be the name of an attribute." << std::endl;
      } else if (classtable->object_table.lookup(name) != NULL) {
        classtable->semant_error(attr)
            << "Attribute " << name << " is multiply defined in class."
            << std::endl;
      } else {
        auto type = classtable->lookup_class(attr->get_type_decl());
        if (type == NULL) {
          classtable->object_table.addid(name, Object_class);
        } else {
          classtable->object_table.addid(name, type);
        }
      }
    }
  }
}

void class__class::exit_scope(ClassTable *classtable) {
  // Exit the scope of the class
  classtable->method_table.exitscope();
  classtable->object_table.exitscope();
  // Exit the scope of the ancestor classes
  auto parent = classtable->lookup_class(get_parent_sym());
  if (parent != NULL) {
    parent->exit_scope(classtable);
  }
}
////////////////////////////////////////////////////////////////////
//
// check_name_and_scope and check_type
//
///////////////////////////////////////////////////////////////////
bool ClassTable::conforms_to(Symbol A, Symbol B) {
  auto A_class = lookup_class(A);
  auto B_class = lookup_class(B);
  if (A_class == NULL || B_class == NULL) {
    return false;
  }
  // A ≤ A for all types A
  if (A == B) {
    return true;
  }
  // if A ≤ C and C ≤ P then A ≤ P
  return conforms_to(A_class->get_parent_sym(), B);
}

bool method_class::is_method() { return true; }
bool attr_class::is_method() { return false; }

void class__class::semant_name_scope(ClassTableP classtable) {
  // 1st pass : installing symbols, from ancestor classes to this class
  install_features(classtable);
  // 2nd pass : undefined symbol/type check
  // 3rd pass : annotate types
  // 4th pass : check types
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    feature->semant_name_scope(classtable);
  }
  exit_scope(classtable);
}

void method_class::semant_name_scope(ClassTableP classtable) {
  bool is_return_type_defined = classtable->lookup_class(return_type) != NULL;
  if (!is_return_type_defined) {
    classtable->semant_error(this) << "Undefined return type " << return_type
                                   << " in method " << name << "." << std::endl;
  }
  // Install the names of the formals in the symbol table
  classtable->object_table.enterscope();
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    auto formal = formals->nth(i);
    formal->semant_name_scope(classtable);
  }
  // Check if expr type matches return_type
  expr->semant_name_scope(classtable);
  // TODO: Only check return_type when the expr has no errors?
  if (is_return_type_defined &&
      !classtable->conforms_to(expr->get_type(), return_type)) {
    classtable->semant_error(this)
        << "Inferred return type " << expr->get_type() << " of method " << name
        << " does not conform to declared return type " << return_type << "."
        << std::endl;
  }
  classtable->object_table.exitscope();
}

void attr_class::semant_name_scope(ClassTableP classtable) {
  init->semant_name_scope(classtable);
  if (classtable->lookup_class(type_decl) == NULL) {
    classtable->semant_error(this) << "Class " << type_decl << " of attribute "
                                   << name << " is undefined." << std::endl;
  }
  // Check if type of init conforms to the type_decl
  // init can be no_expr(), we don't check it then
  if (init->get_type() &&
      !classtable->conforms_to(init->get_type(), type_decl)) {
    classtable->semant_error(this) << "Inferred type " << init->get_type()
                                   << " of initialization of attribute " << name
                                   << " does not conform to declared type "
                                   << type_decl << "." << std::endl;
  }
}

void formal_class::semant_name_scope(ClassTableP classtable) {
  auto type_cls = classtable->lookup_class(type_decl);
  if (name == self) {
    classtable->semant_error(this)
        << "'self' cannot be the name of a formal parameter." << std::endl;
  } else if (classtable->object_table.probe(name) != NULL) {
    classtable->semant_error(this)
        << "Formal parameter " << name << " is multiply defined." << std::endl;
  } else {
    if (type_cls == NULL) {
      classtable->object_table.addid(name, Object_class);
    } else {
      classtable->object_table.addid(name, type_cls);
    }
  }
  if (type_cls == NULL) {
    classtable->semant_error(this)
        << "Class " << type_decl << " of formal parameter " << name
        << " is undefined." << std::endl;
  }
}

void no_expr_class::semant_name_scope(ClassTableP classtable) {}
void isvoid_class::semant_name_scope(ClassTableP classtable) {
  e1->semant_name_scope(classtable);
}
void new__class::semant_name_scope(ClassTableP classtable) {
  auto type = classtable->lookup_class(type_name);
  if (type == NULL) {
    classtable->semant_error(this)
        << "'new' used with undefined class " << type_name << "." << std::endl;
    set_type(Object);
  } else {
    set_type(type_name);
  }
}
void string_const_class::semant_name_scope(ClassTableP classtable) {
  set_type(Str);
}
void bool_const_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
}
void int_const_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
}
void comp_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
}
void leq_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void eq_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void lt_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void neg_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
}
void divide_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void mul_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void sub_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void plus_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
}
void let_class::semant_name_scope(ClassTableP classtable) {
  init->semant_name_scope(classtable);
  auto type_cls = classtable->lookup_class(type_decl);
  if (type_cls == NULL) {
    classtable->semant_error(this)
        << "Class " << type_decl << " of let-bound identifier " << identifier
        << " is undefined." << std::endl;
  }
  // TODO: Check if the type_decl and type(init) matches
  classtable->object_table.enterscope();
  if (identifier == self) {
    classtable->semant_error(this)
        << "'self' cannot be bound in a 'let' expression." << std::endl;
  } else {
    if (type_cls == NULL) {
      classtable->object_table.addid(identifier, Object_class);
    } else {
      classtable->object_table.addid(identifier, type_cls);
    }
  }
  body->semant_name_scope(classtable);
  set_type(body->get_type());
  classtable->object_table.exitscope();
}
void block_class::semant_name_scope(ClassTableP classtable) {
  // TODO: Empty block check
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    body->nth(i)->semant_name_scope(classtable);
    // Set type of the last expression
    set_type(body->nth(i)->get_type());
  }
}
Symbol ClassTable::join_type(Symbol s1, Symbol s2) {
  // case 1 : s1 is a subclass of s2
  if (conforms_to(s1, s2)) {
    return s2;
  }
  // case 2 : s2 is a subclass of s1
  if (conforms_to(s2, s1)) {
    return s1;
  }
  // case 3 : s1 and s2 has a common ancestor
  auto s1_class = lookup_class(s1);
  auto s2_class = lookup_class(s2);
  return join_type(s1_class->get_parent_sym(), s2_class->get_parent_sym());
}
void typcase_class::semant_name_scope(ClassTableP classtable) {
  // TODO: Empty case check
  expr->semant_name_scope(classtable);
  classtable->branch_table.enterscope();
  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    auto branch = cases->nth(i);
    branch->semant_name_scope(classtable);
    if (type == NULL) {
      set_type(branch->get_type());
    } else {
      set_type(classtable->join_type(branch->get_type(), get_type()));
    }
  }
  classtable->branch_table.exitscope();
}

void branch_class::semant_name_scope(ClassTableP classtable) {
  classtable->object_table.enterscope();
  auto type = classtable->lookup_class(type_decl);
  // Add the branch name to the object table (variable)
  if (name == self) {
    classtable->semant_error(this) << "'self' bound in 'case'." << std::endl;
  } else {
    if (type == NULL) {
      classtable->object_table.addid(name, Object_class);
    } else {
      classtable->object_table.addid(name, type);
    }
  }
  // Add the branch to the branch table (type)
  if (type == NULL) {
    classtable->semant_error(this)
        << "Class " << type_decl << " of case-bound identifier " << name
        << " is undefined." << std::endl;
  } else if (classtable->branch_table.probe(type_decl) != NULL) {
    classtable->semant_error(this) << "Duplicate branch " << type_decl
                                   << " in case statement." << std::endl;
  } else {
    classtable->branch_table.addid(type_decl, type);
  }
  expr->semant_name_scope(classtable);
  classtable->object_table.exitscope();
}
void loop_class::semant_name_scope(ClassTableP classtable) {
  set_type(Object);
  pred->semant_name_scope(classtable);
  body->semant_name_scope(classtable);
}
void cond_class::semant_name_scope(ClassTableP classtable) {
  pred->semant_name_scope(classtable);
  // TODO: Check if pred is of type Bool
  then_exp->semant_name_scope(classtable);
  else_exp->semant_name_scope(classtable);
  // TODO: Set type (join of then and else)
}
void dispatch_class::semant_name_scope(ClassTableP classtable) {
  expr->semant_name_scope(classtable);
  auto expr_type = expr->get_type();
  // Check if name is a method of the class of the expr
  auto expr_class = classtable->lookup_class(expr_type);
  auto method = classtable->lookup_method(expr_class, name);
  set_type(Object);
  if (method == NULL) {
    classtable->semant_error(this)
        << "Dispatch to undefined method " << name << "." << std::endl;
  } else {
    // Set type of the method
    set_type(method->get_return_type());
    // TODO: We need to set the type of the method's class
    if (get_type() == SELF_TYPE) {
      set_type(expr_type);
    }
  }
  if (method && actual->len() != method->get_formals()->len()) {
    classtable->semant_error(this)
        << "Method " << name << " called with wrong number of arguments."
        << std::endl;
  } else {
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
      auto actual_expr = actual->nth(i);
      actual_expr->semant_name_scope(classtable);
      if (method) {
        auto formal = method->get_formals()->nth(i);
        auto actual_type = actual_expr->get_type();
        auto formal_type = formal->get_type_decl();
        // Check if the actual type conforms to the formal type
        if (!classtable->conforms_to(actual_type, formal_type)) {
          classtable->semant_error(this)
              << "In call of method " << name << ", type " << actual_type
              << " of parameter " << formal->get_name()
              << " does not conform to declared type " << formal_type << "."
              << std::endl;
        }
      }
    }
  }
}

void static_dispatch_class::semant_name_scope(ClassTableP classtable) {
  // Check expr conforms to the type_name : e.g. `(new C)@B.f()`, where B should
  // be the ancestor of C.
  expr->semant_name_scope(classtable);
  auto left_class = classtable->lookup_class(expr->get_type());
  auto right_class = classtable->lookup_class(type_name);
  auto method = classtable->lookup_method(right_class, name);
  if (!classtable->conforms_to(expr->get_type(), type_name)) {
    classtable->semant_error(this)
        << "Expression type " << expr->get_type()
        << " does not conform to declared static dispatch type " << type_name
        << "." << std::endl;
    set_type(Object);
  } else {
    // Check if name is a method of the parent class
    if (method == NULL) {
      classtable->semant_error(this)
          << "Dispatch to undefined method " << name << "." << std::endl;
    } else {
      set_type(method->get_return_type());
      // TODO: We need to set the type of the method's class
      if (get_type() == SELF_TYPE) {
        set_type(expr->get_type());
      }
    }
  }

  if (method && actual->len() != method->get_formals()->len()) {
    classtable->semant_error(this)
        << "Method " << name << " called with wrong number of arguments."
        << std::endl;
  } else {
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
      auto actual_expr = actual->nth(i);
      actual_expr->semant_name_scope(classtable);
      if (method) {
        auto formal = method->get_formals()->nth(i);
        auto actual_type = actual_expr->get_type();
        auto formal_type = formal->get_type_decl();
        // Check if the actual type conforms to the formal type
        if (!classtable->conforms_to(actual_type, formal_type)) {
          classtable->semant_error(this)
              << "In call of method " << name << ", type " << actual_type
              << " of parameter " << formal->get_name()
              << " does not conform to declared type " << formal_type << "."
              << std::endl;
        }
      }
    }
  }
}

method_class *ClassTable::lookup_method(Class_ cls, Symbol name) {
  if (cls == NULL) {
    return NULL;
  }
  auto method = cls->lookup_method(name);
  if (method != NULL) {
    return method;
  }
  auto parent = lookup_class(cls->get_parent_sym());
  return lookup_method(parent, name);
}

method_class *class__class::lookup_method(Symbol name) {
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    if (feature->is_method() && feature->get_name() == name) {
      return dynamic_cast<method_class *>(feature);
    }
  }
  return NULL;
}

void assign_class::semant_name_scope(ClassTableP classtable) {
  auto identifier_cls = classtable->object_table.lookup(name);
  set_type(Object);
  if (identifier_cls == NULL) {
    classtable->semant_error(this)
        << "Assignment to undeclared variable " << name << "." << std::endl;
  } else {
    // Set type of the identifier
    if (name == self) {
      classtable->semant_error(this) << "Cannot assign to 'self'." << std::endl;
    }
    expr->semant_name_scope(classtable);
    set_type(expr->get_type());
    // Check if the expr conforms to identifier type
    if (!classtable->conforms_to(expr->get_type(),
                                 identifier_cls->get_name())) {
      // TODO: This is to mimic reference coolc implementation
      // But I don't think it's good way. They may have different
      // implementation:
      //   object_table[self] = SELF_TYPE // this may be reference
      //   implementation object_table[self] = class A   // this is my
      //   implementation
      // But, then, how SELF_TYPE is resolved when it's needed to?
      // Simply classtable->visiting can resolve this?
      auto declared_type = identifier_cls->get_name();
      if (name == self) {
        declared_type = SELF_TYPE;
      }
      classtable->semant_error(this)
          << "Type " << expr->get_type()
          << " of assigned expression does not conform to declared type "
          << declared_type << " of identifier " << name << "." << std::endl;
    }
  }
}

void object_class::semant_name_scope(ClassTableP classtable) {
  auto object_type = classtable->object_table.lookup(name);
  if (object_type == NULL) {
    classtable->semant_error(this)
        << "Undeclared identifier " << name << "." << std::endl;
    set_type(Object);
  } else {
    set_type(object_type->get_name());
  }
}

void ClassTable::semant_name_scope(Classes classes) {
  if (semant_debug) {
    std::cout << "Checking naming and scoping..." << std::endl;
  }
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    visiting = classes->nth(i);
    class_table.enterscope();
    visiting->semant_name_scope(this);
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
  /* Check Cyclic Inheritance */
  if (classtable->errors() == 0) {
    classtable->semant_cyclic_inheritance(classes);
  }
  /* Check Naming and Scoping */
  if (classtable->errors() == 0) {
    classtable->semant_name_scope(classes);
  }
  /* Type Checking */
  if (classtable->errors() == 0) {
    classtable->check_type();
  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
}

Class_ ClassTable::lookup_class(Symbol name) {
  return class_table.lookup(name);
}
