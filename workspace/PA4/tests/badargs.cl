class Main { main() : Object {0}; };

class A {
      bar() : Object {
        {
	    (new B).foo(new A, 29);
	    (new B).foo(new A, 29);
	    (new B).foo(new B, 29); (* This is legal *)
	    (new B).foo(new B, 29, 42); (* Too many arguments *)
	    (new B).foo(self, 29, 42); (* Too many arguments & wrong type *)
	    (new B).foo(new B); (* Too few arguments *)
	    (new B).foo(self); (* Too few arguments & wrong type *)
        }
      };
};

class B inherits A {
      foo(b:B, x:Int) : String { "moo" };
};


