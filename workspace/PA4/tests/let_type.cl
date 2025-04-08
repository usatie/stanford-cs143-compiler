class Main { main(): Int {42}; };

class A {
  x: Int <- let x: String in 42;
  x: String <- let x: Int in "hello";
};
