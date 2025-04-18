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
static Class_ Object_class;
// TODO: Refactor this as non-global variable later
static TypeEnvironment *env;

// Forward declarations
method_class *lookup_method_in_hierarchy(Class_ cls, Symbol name);
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

  /* Add basic classes to the class table */
  class_table.enterscope();
  class_table.addid(Object, Object_class);
  class_table.addid(IO, IO_class);
  class_table.addid(Int, Int_class);
  class_table.addid(Bool, Bool_class);
  class_table.addid(Str, Str_class);
  class_table.addid(SELF_TYPE, (Class_)1); // SELF_TYPE is a dummy class
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {

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

////////////////////////////////////////////////////////////////////
//
// TypeEnvironment is a class that contains the symbol table for the
// type environment. It is used to store the types of the variables and
// attributes in the class hierarchy.
//
//   1. Object Environment O
//   2. Method Environment M
//   3. Current Class C
//
///////////////////////////////////////////////////////////////////
TypeEnvironment::TypeEnvironment(ClassTableP classtable)
    : classtable(classtable), o(), m(), c(nullptr) {
  o.enterscope();
  m.enterscope();
  o.addid(self, SELF_TYPE);
}

TypeEnvironment::~TypeEnvironment() {
  o.exitscope();
  m.exitscope();
}

ostream &TypeEnvironment::semant_error(tree_node *t) {
  Class_ cls = dynamic_cast<Class_>(t);
  if (cls != NULL) {
    return classtable->semant_error(cls);
  }
  return classtable->semant_error(c->get_filename(), t);
}

Class_ TypeEnvironment::lookup_class(Symbol name) {
  if (name == SELF_TYPE) {
    return c;
  }
  return classtable->class_table.lookup(name);
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
  Symbol parent_sym = c->get_parent();
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
  auto parent = class_table.lookup(curr->get_parent());
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
// lookup_method is a method that looks up a method in the class
// hierarchy.
//
// method_class *lookup_method(Class_ cls, Symbol name)
//   Returns the method with the given name in the class hierarchy.
//
// method_class *class__class::lookup_method(Symbol name)
//   Looks up a method in the class. It is used to find the
//   method in the class hierarchy.
//
///////////////////////////////////////////////////////////////////
method_class *lookup_method_in_hierarchy(Class_ cls, Symbol name) {
  if (cls == NULL) {
    return NULL;
  }
  auto method = cls->lookup_method(name);
  if (method != NULL) {
    return method;
  }
  auto parent = env->lookup_class(cls->get_parent());
  return lookup_method_in_hierarchy(parent, name);
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

////////////////////////////////////////////////////////////////////
//
// Install features
//
///////////////////////////////////////////////////////////////////
static void install_method(ClassTableP classtable, method_class *method,
                           method_class *overrided_method) {
  auto name = method->get_name();
  if (env->m.probe(name) != NULL) {
    env->semant_error(method)
        << "Method " << name << " is multiply defined." << std::endl;
    return;
  }
  if (overrided_method != NULL) {
    auto overrided_formals = overrided_method->get_formals();
    auto method_formals = method->get_formals();
    // 1. Check return type
    if (overrided_method->get_return_type() != method->get_return_type()) {
      env->semant_error(method)
          << "In redefined method " << name << ", return type "
          << method->get_return_type()
          << " is different from original return type "
          << overrided_method->get_return_type() << "." << std::endl;
      return;
    }
    // 2. Check number of formals
    if (overrided_formals->len() != method_formals->len()) {
      env->semant_error(method)
          << "Incompatible number of formal parameters in redefined method "
          << name << "." << std::endl;
      return;
    }
    // 3. Check formal types
    for (int i = overrided_formals->first(), j = method_formals->first();
         overrided_formals->more(i) && method_formals->more(j);
         i = overrided_formals->next(i), j = method_formals->next(j)) {
      auto overrided_type = overrided_formals->nth(i)->get_type_decl();
      auto method_type = method_formals->nth(j)->get_type_decl();
      if (overrided_type != method_type) {
        env->semant_error(method)
            << "In redefined method " << name << ", parameter type "
            << method_type << " is different from original type "
            << overrided_type << std::endl;
        return;
      }
    }
  }
  env->m.addid(name, method);
}
static void install_attribute(ClassTableP classtable, attr_class *attr) {
  auto name = attr->get_name();
  if (name == self) {
    env->semant_error(attr)
        << "'self' cannot be the name of an attribute." << std::endl;
  } else if (env->o.lookup(name) != NULL) {
    env->semant_error(attr) << "Attribute " << name
                            << " is multiply defined in class." << std::endl;
  } else {
    auto type = env->lookup_class(attr->get_type_decl());
    if (type == NULL) {
      env->o.addid(name, Object);
    } else {
      env->o.addid(name, attr->get_type_decl());
    }
  }
}
void class__class::install_features(ClassTable *classtable) {
  // Install symbols from anscestor classes
  auto parent = env->lookup_class(get_parent());
  if (parent != NULL) {
    parent->install_features(classtable);
  }
  // Install symbols from features
  // Methods:
  //   1. Allow overriding ancestor methods
  //   2. however, duplicate methods are not allowed
  // Attributes:
  //   1. Inherited attributes cannot be redefined.
  env->m.enterscope();
  env->o.enterscope();
  for (int i = features->first(); features->more(i); i = features->next(i)) {
    auto feature = features->nth(i);
    if (feature->is_method()) {
      auto overrided_method =
          lookup_method_in_hierarchy(parent, feature->get_name());
      install_method(classtable, dynamic_cast<method_class *>(feature),
                     overrided_method);
    } else {
      install_attribute(classtable, dynamic_cast<attr_class *>(feature));
    }
  }
}

void class__class::exit_scope(ClassTable *classtable) {
  // Exit the scope of the class
  env->m.exitscope();
  env->o.exitscope();
  // Exit the scope of the ancestor classes
  auto parent = env->lookup_class(get_parent());
  if (parent != NULL) {
    parent->exit_scope(classtable);
  }
}

////////////////////////////////////////////////////////////////////
//
// Type Annotation and Type Checking
//
// bool conforms_to(Symbol A, Symbol B)
//   Checks if class A conforms to class B. It is used to check if
//   a class is a subclass of another class.
//
// Symbol lub(Symbol s1, Symbol s2)
//   Returns the least upper bound (common ancestor) of two
//   classes. It is used to determine the static type of joining
//   two classes.
//
///////////////////////////////////////////////////////////////////
// A <= B if A is a subclass of B
bool conforms_to(Symbol A, Symbol B) {
  if (A == No_class) {
    return false;
  }
  // 1. Contains SELF_TYPE in comparison
  // 1-1. SELF_TYPEc <= SELF_TYPEc
  if (A == SELF_TYPE && B == SELF_TYPE) {
    return true;
  }
  // 1-2. SELF_TYPEc <= T if C <= T
  if (A == SELF_TYPE) {
    return conforms_to(env->lookup_class(A)->get_name(), B);
  }
  // 1-3. T <= SELF_TYPEc is always false
  if (B == SELF_TYPE) {
    return false;
  }
  // 2. Normal case
  // 2-1. X ≤ X
  if (A == B) {
    return true;
  }
  // 2-2. X <= Y if X inherits from Y
  auto parent = env->lookup_class(A)->get_parent();
  if (parent == B) {
    return true;
  }
  // 2-3. X <= Z if X ≤ Y and Y ≤ Z
  //     (A <= parent && parent <= B)
  return conforms_to(parent, B);
}

Symbol lub(Symbol s1, Symbol s2) {
  // 1-1. lub(SELF_TYPEc, SELF_TYPEc) = SELF_TYPEc
  if (s1 == SELF_TYPE && s2 == SELF_TYPE) {
    return SELF_TYPE;
  }
  // 1-2. lub(SELF_TYPEc, T) = lub(C, T)
  if (s1 == SELF_TYPE) {
    return lub(env->lookup_class(s1)->get_name(), s2);
  }
  // 1-3. lub(T, SELF_TYPEc) = lub(C, T)
  if (s2 == SELF_TYPE) {
    return lub(env->lookup_class(s2)->get_name(), s1);
  }
  while (1) {
    // case 1 : s1 is a subclass of s2
    if (conforms_to(s1, s2)) {
      return s2;
    }
    // case 2 : s2 is a subclass of s1
    if (conforms_to(s2, s1)) {
      return s1;
    }
    // case 3 : s1 and s2 has a common ancestor
    s1 = env->lookup_class(s1)->get_parent();
    s2 = env->lookup_class(s2)->get_parent();
  }
}

////////////////////////////////////////////////////////////////////
//
// is_method is a method that checks if the feature is a method or an
// attribute. It is used to distinguish between methods and
// attributes in the class hierarchy.
//
// bool method_class::is_method()
// bool attr_class::is_method()
//
///////////////////////////////////////////////////////////////////
bool method_class::is_method() { return true; }
bool attr_class::is_method() { return false; }

////////////////////////////////////////////////////////////////////
//
// semant_name_scope is a method that performs semantic analysis on the
// class hierarchy. It is implemented for each AST node type.
//
// void ::semant_name_scope(ClassTableP classtable)
//
///////////////////////////////////////////////////////////////////
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
  bool is_return_type_defined = env->lookup_class(return_type) != NULL;
  if (!is_return_type_defined) {
    env->semant_error(this) << "Undefined return type " << return_type
                            << " in method " << name << "." << std::endl;
  }
  // Install the names of the formals in the symbol table
  env->o.enterscope();
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    auto formal = formals->nth(i);
    formal->semant_name_scope(classtable);
  }
  // Check if expr type matches return_type
  expr->semant_name_scope(classtable);
  // TODO: Only check return_type when the expr has no errors?
  if (is_return_type_defined && !conforms_to(expr->get_type(), return_type)) {
    env->semant_error(this)
        << "Inferred return type " << expr->get_type() << " of method " << name
        << " does not conform to declared return type " << return_type << "."
        << std::endl;
  }
  env->o.exitscope();
}

void attr_class::semant_name_scope(ClassTableP classtable) {
  init->semant_name_scope(classtable);
  if (env->lookup_class(type_decl) == NULL) {
    env->semant_error(this) << "Class " << type_decl << " of attribute " << name
                            << " is undefined." << std::endl;
  }
  // Check if type of init conforms to the type_decl
  // init can be no_expr(), we don't check it then
  if (init->get_type() && !conforms_to(init->get_type(), type_decl)) {
    env->semant_error(this) << "Inferred type " << init->get_type()
                            << " of initialization of attribute " << name
                            << " does not conform to declared type "
                            << type_decl << "." << std::endl;
  }
}

void formal_class::semant_name_scope(ClassTableP classtable) {
  auto type_cls = env->lookup_class(type_decl);
  if (name == self) {
    env->semant_error(this)
        << "'self' cannot be the name of a formal parameter." << std::endl;
  } else if (env->o.probe(name) != NULL) {
    env->semant_error(this)
        << "Formal parameter " << name << " is multiply defined." << std::endl;
  } else {
    if (type_cls == NULL) {
      env->o.addid(name, Object);
    } else {
      env->o.addid(name, type_decl);
    }
  }
  if (type_decl == SELF_TYPE) {
    env->semant_error(this) << "Formal parameter " << name
                            << " cannot have type SELF_TYPE." << std::endl;
  } else if (type_cls == NULL) {
    env->semant_error(this) << "Class " << type_decl << " of formal parameter "
                            << name << " is undefined." << std::endl;
  }
}

void no_expr_class::semant_name_scope(ClassTableP classtable) {}

void isvoid_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
}

void new__class::semant_name_scope(ClassTableP classtable) {
  auto type = env->lookup_class(type_name);
  if (type == NULL) {
    env->semant_error(this)
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
  if (e1->get_type() != Bool) {
    env->semant_error(this) << "Argument of 'not' has type " << e1->get_type()
                            << " instead of Bool." << std::endl;
  }
}

void leq_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type()
                            << " <= " << e2->get_type() << std::endl;
  }
}

static bool is_basic_type(Symbol type) {
  return type == Int || type == Bool || type == Str;
}

void eq_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (is_basic_type(e1->get_type()) || is_basic_type(e2->get_type())) {
    if (e1->get_type() != e2->get_type()) {
      env->semant_error(this)
          << "Illegal comparison with a basic type." << std::endl;
    }
  }
}

void lt_class::semant_name_scope(ClassTableP classtable) {
  set_type(Bool);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type() << " < "
                            << e2->get_type() << std::endl;
  }
}

void neg_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  if (e1->get_type() != Int) {
    env->semant_error(this) << "Argument of '~' has type " << e1->get_type()
                            << " instead of Int." << std::endl;
  }
}

void divide_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type() << " / "
                            << e2->get_type() << std::endl;
  }
}

void mul_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type() << " * "
                            << e2->get_type() << std::endl;
  }
}

void sub_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type() << " - "
                            << e2->get_type() << std::endl;
  }
}

void plus_class::semant_name_scope(ClassTableP classtable) {
  set_type(Int);
  e1->semant_name_scope(classtable);
  e2->semant_name_scope(classtable);
  if (e1->get_type() != Int || e2->get_type() != Int) {
    env->semant_error(this) << "non-Int arguments: " << e1->get_type() << " + "
                            << e2->get_type() << std::endl;
  }
}

void let_class::semant_name_scope(ClassTableP classtable) {
  // 1. Check Undefined type_decl
  auto identifier_type = type_decl;
  if (env->lookup_class(type_decl) == NULL) {
    env->semant_error(this)
        << "Class " << type_decl << " of let-bound identifier " << identifier
        << " is undefined." << std::endl;
    identifier_type = Object; // Set to Object if undefined
  }
  // 2. Check init expression
  init->semant_name_scope(classtable);
  // 3. Check if the type_decl and init.type matches (Only check if init is not
  // no_expr)
  bool init_omitted = init->get_type() == NULL; // no_expr does not have a type
  if (!init_omitted && !conforms_to(init->get_type(), identifier_type)) {
    env->semant_error(this)
        << "Inferred type " << init->get_type() << " of initialization of "
        << identifier << " does not conform to identifier's declared type "
        << identifier_type << "." << std::endl;
  }
  // 4. Add identifier to object table
  env->o.enterscope();
  if (identifier == self) {
    env->semant_error(this)
        << "'self' cannot be bound in a 'let' expression." << std::endl;
  } else {
    env->o.addid(identifier, identifier_type);
  }
  // 5. Check body expression
  body->semant_name_scope(classtable);
  env->o.exitscope();
  set_type(body->get_type());
}

void block_class::semant_name_scope(ClassTableP classtable) {
  // Empty block check is not necessary (because it is handled in the parser)
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    body->nth(i)->semant_name_scope(classtable);
    // Set type of the last expression
    set_type(body->nth(i)->get_type());
  }
}

void typcase_class::semant_name_scope(ClassTableP classtable) {
  // Empty case check is not necessary (because it is handled in the parser)
  expr->semant_name_scope(classtable);
  classtable->branch_table.enterscope();
  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    auto branch = cases->nth(i);
    branch->semant_name_scope(classtable);
    if (type == NULL) {
      set_type(branch->get_type());
    } else {
      set_type(lub(branch->get_type(), get_type()));
    }
  }
  classtable->branch_table.exitscope();
}

void branch_class::semant_name_scope(ClassTableP classtable) {
  env->o.enterscope();
  auto type = env->lookup_class(type_decl);
  // Add the branch name to the object table (variable)
  if (name == self) {
    env->semant_error(this) << "'self' bound in 'case'." << std::endl;
  } else {
    if (type == NULL) {
      env->o.addid(name, Object);
    } else {
      env->o.addid(name, type_decl);
    }
  }
  // Add the branch to the branch table (type)
  if (type == NULL) {
    env->semant_error(this)
        << "Class " << type_decl << " of case-bound identifier " << name
        << " is undefined." << std::endl;
  } else if (classtable->branch_table.probe(type_decl) != NULL) {
    env->semant_error(this) << "Duplicate branch " << type_decl
                            << " in case statement." << std::endl;
  } else {
    classtable->branch_table.addid(type_decl, type);
  }
  // Check illegal SELF_TYPE usage
  if (type_decl == SELF_TYPE) {
    env->semant_error(this)
        << "Identifier " << name
        << " declared with type SELF_TYPE in case branch." << std::endl;
  }
  expr->semant_name_scope(classtable);
  env->o.exitscope();
}

void loop_class::semant_name_scope(ClassTableP classtable) {
  set_type(Object);
  pred->semant_name_scope(classtable);
  body->semant_name_scope(classtable);
  if (pred->get_type() != Bool) {
    env->semant_error(this)
        << "Loop condition does not have type Bool." << std::endl;
  }
}

void cond_class::semant_name_scope(ClassTableP classtable) {
  if (semant_debug) {
    env->semant_error(this) << "cond_class::semant_name_scope " << std::endl;
  }
  pred->semant_name_scope(classtable);
  // Check if pred is of type Bool
  if (pred->get_type() != Bool) {
    env->semant_error(this)
        << "Predicate of 'if' does not have type Bool." << std::endl;
  }
  then_exp->semant_name_scope(classtable);
  else_exp->semant_name_scope(classtable);
  // Set type (join of then and else)
  set_type(lub(then_exp->get_type(), else_exp->get_type()));
}

void dispatch_class::semant_name_scope(ClassTableP classtable) {
  expr->semant_name_scope(classtable);
  auto expr_type = expr->get_type();
  // Check if name is a method of the class of the expr
  auto expr_class = env->lookup_class(expr_type);
  auto method = lookup_method_in_hierarchy(expr_class, name);
  set_type(Object);
  if (method == NULL) {
    env->semant_error(this)
        << "Dispatch to undefined method " << name << "." << std::endl;
  } else {
    // Set type of the method
    set_type(method->get_return_type());
    if (get_type() == SELF_TYPE) {
      set_type(expr_type);
    }
  }
  if (method && actual->len() != method->get_formals()->len()) {
    env->semant_error(this)
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
        if (!conforms_to(actual_type, formal_type)) {
          env->semant_error(this)
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
  auto left_class = env->lookup_class(expr->get_type());
  auto right_class = env->lookup_class(type_name);
  auto method = lookup_method_in_hierarchy(right_class, name);
  set_type(Object);
  if (type_name == SELF_TYPE) {
    env->semant_error(this) << "Static dispatch to SELF_TYPE." << std::endl;
  } else if (!conforms_to(expr->get_type(), type_name)) {
    env->semant_error(this)
        << "Expression type " << expr->get_type()
        << " does not conform to declared static dispatch type " << type_name
        << "." << std::endl;
  } else if (method == NULL) {
    env->semant_error(this)
        << "Dispatch to undefined method " << name << "." << std::endl;
  } else {
    // Check if name is a method of the parent class
    set_type(method->get_return_type());
    if (get_type() == SELF_TYPE) {
      set_type(expr->get_type());
    }
  }

  if (method && actual->len() != method->get_formals()->len()) {
    env->semant_error(this)
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
        if (!conforms_to(actual_type, formal_type)) {
          env->semant_error(this)
              << "In call of method " << name << ", type " << actual_type
              << " of parameter " << formal->get_name()
              << " does not conform to declared type " << formal_type << "."
              << std::endl;
        }
      }
    }
  }
}

void assign_class::semant_name_scope(ClassTableP classtable) {
  // 1. Check the identifier
  auto declared_type = env->o.lookup(name);
  if (name == self) {
    env->semant_error(this) << "Cannot assign to 'self'." << std::endl;
  } else if (declared_type == NULL) {
    env->semant_error(this)
        << "Assignment to undeclared variable " << name << "." << std::endl;
    declared_type = Object; // Default to Object if not found
  }
  // 2. Check the expression
  expr->semant_name_scope(classtable);
  set_type(expr->get_type());
  // 3. Check the type conformance (expr <= declared_type)
  if (!conforms_to(expr->get_type(), declared_type)) {
    env->semant_error(this)
        << "Type " << expr->get_type()
        << " of assigned expression does not conform to declared type "
        << declared_type << " of identifier " << name << "." << std::endl;
  }
}

void object_class::semant_name_scope(ClassTableP classtable) {
  auto declared_type = env->o.lookup(name);
  if (declared_type == NULL) {
    env->semant_error(this)
        << "Undeclared identifier " << name << "." << std::endl;
    set_type(Object);
  } else {
    set_type(declared_type);
  }
}

////////////////////////////////////////////////////////////////////
//
// Semantic analysis
//
// semant_main is a method that checks the existence of the
// Main class and the main method. It is used to
// check the entry point of the program.
//
// semant_name_scope is a method that performs semantic analysis
// on the class hierarchy. It is implemented for each AST node
// type.
//
///////////////////////////////////////////////////////////////////
static void semant_main() {
  if (semant_debug) {
    std::cout << "Checking main..." << std::endl;
  }
  auto c = env->lookup_class(Main);
  if (c == NULL) {
    env->semant_error() << "Class Main is not defined." << std::endl;
    return;
  }
  auto m = c->lookup_method(main_meth);
  if (m == NULL) {
    env->semant_error(c) << "No 'main' method in class Main." << std::endl;
    return;
  }
  if (m->get_formals()->len() != 0) {
    env->semant_error(c)
        << "'main' method in class Main should have no arguments." << std::endl;
  }
}

static void semant_name_scope(Classes classes) {
  if (semant_debug) {
    std::cout << "Checking naming and scoping..." << std::endl;
  }
  /* Check the existence of class Main */
  semant_main();

  /* Naming and Scoping check for each class */
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    auto c = classes->nth(i);
    env->c = c;
    c->semant_name_scope(env->classtable);
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
  env = new TypeEnvironment(classtable);

  /* some semantic analysis code may go here */

  /* Check Cyclic Inheritance */
  if (env->errors() == 0) {
    classtable->semant_cyclic_inheritance(classes);
  }
  /* Check Naming and Scoping */
  if (env->errors() == 0) {
    semant_name_scope(classes);
  }

  if (env->errors()) {
    delete env;
    delete classtable;
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
  delete env;
  delete classtable;
}
