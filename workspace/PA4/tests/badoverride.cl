class Main { main() : Int {0 }; };

class A {};
class B inherits A {};
class C inherits B {};

class D {
	f(b: B):Object { 1 };
	g(a: A, b: B):Int { 1 };
	h(b: B):Object { 1 };
};

class E inherits D {
	f(b: B):Int { 1 };    (* 1. return type *)
	g():Int { 1 };        (* 2. number of parameters *)
	h(b: C):Object { 1 }; (* 3. formal type *)
};

class F inherits D {
	g(a: B):Object { 1 }; (* 1 & 2 & 3 : Only return type(1) is checked *)
	h():Int { 1 };        (* 1 & 2 : Only return type(1) is checked *)
};

class G inherits D {
	h(a: B, b: C):Int { 1 };  (* 3 : Only the first formal(2) is checked *)
    h(a: B, b: C): Int { 1 }; (* If multiple definition, no check is performed *)
};

class G inherits D {
	h(a: B, b: C, c: D):Int { 1 }; (* 1 & 2 : Only the first formal(2) is checked *)
};
