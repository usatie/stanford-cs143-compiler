(* Bad Inheritance Check *)
class A inherits Bool {
  x: U;
};
class B inherits Int {
  x: U;
};
class C inherits String {};
class D inherits SELF_TYPE {};
class E inherits U {};
class F inherits U {};
class X inherits IO {}; (* This is legal *)
class Y inherits Object {}; (* This is legal *)

(* Redefinition check is actually done before the above check *)
class FOO {};
class FOO {};
class FOO {};
class Bool {};

(* Cyclic inheritance check is only done if the above check passes *)
class AAA inherits BBB {};
class BBB inherits CCC {};
class CCC inherits AAA {};
