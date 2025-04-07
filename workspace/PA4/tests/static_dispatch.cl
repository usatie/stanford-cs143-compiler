class Main { main(): Int {42}; };

class A {
  f(): Int {42};
};

class B inherits A {
  g(): Int {21};
};

class C inherits B {
  h(): Int {
    {
      (new C)@B.f() + (new C).g();
      (new C)@D.f();
    }
  };
};

class D inherits A {};
