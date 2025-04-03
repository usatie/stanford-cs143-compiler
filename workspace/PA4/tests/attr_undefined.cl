class Main { main() : Object {0}; };

class Cow { 
      moo : Object;
};

class Bird {
      f(): Int { 42 };
      x(obj: Object) : Int { 42 };
};

class Chicken inherits Bird {
      sound : Object <- moo;
      cow: Object <- new Pig;
      a1: Int <- 42 + x(moo);
      a2: Int <- 42 - x(moo);
      a3: Int <- 42 * x(moo);
      a4: Int <- 42 / x(moo);
      a5: Int <- ~ x(moo);
      a6: Bool <- 42 = x(moo);
      a7: Bool <- 42 < x(moo);
      a8: Bool <- 42 <= x(moo);
      a9: Int <- { x(moo); };
      a10: Int <- if x(moo) = 42 then 42 else 0 fi;
      a11: Int <- if true then x(moo) else 0 fi;
      a12: Int <- if true then 42 else x(moo) fi;
      a13: Object <- while true loop x(moo) pool;
      a14: Object <- while x(moo) = 42 loop 42 pool;
      a15: Int <- x(moo);
      a16: Int <- self.x(moo);
      a17: Int <- self@Bird.x(moo);
      a18: Object <- moo.copy();
      a19: Object <- moo@Object.copy();
      a20: Object <- sound <- moo;
      a21: Object <- moo <- sound;

      f(): Int { 42 };
      x(obj: Object) : Int { 42 };
};
