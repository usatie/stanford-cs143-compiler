class Main { main(x: Int): Int {x}; };

(* Name and Scoping check will be done even if the above check fails *)
class A {
  x: Object <- foo;
  y: Int <- x;
};
