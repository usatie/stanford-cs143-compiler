class Main { main(): Int {42}; };

class B {};

class A {
    a: Int;
    self: Int; (* attribute `self` : 1st pass *)
    f(self: Int): Int { 42 }; (* Formal parameter `self`: 2nd pass *)
    a: Int <- let self: Int in 42; (* Let-bound `self` : 2nd pass *)
    g(x: Hoge): Int { 42 }; (* Undefined type : 2nd pass *)
    self(): Int { 42 }; (* Method name `self` is actually legal *)
    a: Int; (* multiple definition: 1st pass *)
    x:A <- self; (* This is legal *)
    foo(x:A) : A {self <- (new B).copy()}; (* illegal: self is not a variable *)
    z: Int <- case 42 of
      self: Int => 4242;
      self: A => 4343;
    esac; (* case-bound `self` : 2nd pass *)
};
