class Main { main(): Int {42}; };

class A {
  f(x: Int): SELF_TYPE {
    {
      f(self);
      f({self;});
    }
  };
  g(): Int { self };
};

class B inherits A {
  aa: A <- (new A).f(42); (* Type match    : Inferred type is A *)
  ab: A <- (new B).f(42); (* Type match    : Inferred type is B *)
  ac: A <- (new C).f(42); (* Type match    : Inferred type is C *)
  ba: B <- (new A).f(42); (* Type mismatch : Inferred type is A *)
  bb: B <- (new B).f(42); (* Type match    : Inferred type is B *)
  bc: B <- (new C).f(42); (* Type mismatch : Inferred type is C *)
  ca: C <- (new A).f(42); (* Type mismatch : Inferred type is A *)
  cb: C <- (new B).f(42); (* Type mismatch : Inferred type is B *)
  cc: C <- (new C).f(42); (* Type match    : Inferred type is C *)
  oacopy: Object <- (new A).copy(); (* Type match    : Inferred type is A *)
  aacopy: A <- (new A).copy();      (* Type match    : Inferred type is A *)
  bacopy: B <- (new A).copy();      (* Type mismatch : Inferred type is A *)
  obcopy: Object <- (new B).copy(); (* Type match    : Inferred type is B *)
  abcopy: A <- (new B).copy();      (* Type match    : Inferred type is B *)
  bbcopy: B <- (new B).copy();      (* Type match    : Inferred type is B *)
};
class C inherits A {};
class D {};
