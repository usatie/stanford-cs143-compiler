class Main { main() : Int {0 }; };

class A {};
class B inherits A {};
class C inherits B {};

class D {
	f(b:B):Object { 1 };
	g(b:B):Object { 1 };
	h(b:B):Object { 1 };
};

class E inherits D {
	f(b:C):Object { 1 };
	g(b:B):Int { 1 };
	h():Object { 1 };
};
