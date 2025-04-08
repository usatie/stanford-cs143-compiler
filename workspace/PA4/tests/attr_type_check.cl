class Main { main(): Int {42}; };

class A {
  f(): Int { 42 };
  s: String <- (new A).f();
};
