class Main { 
  main() : Object {
   {
     let a: U in 10; (* 1. undefined type_decl *)
     let a: Object <- {z;} in 10; (* 2. error in init expr *)
     let a: B <- new A in 10; (* 3. Non-conforming *)
     let self: A in 10; (* 4. self in let *)
     let b: Object in {z;}; (* 5. error in body expr *)
     (* Combination of 1~5 *)
     let self: U in 10; (* 1 & 4 *)
     let self: B <- {x; new A;} in {y;}; (* 2 & 3 & 4 & 5*)
     let self: U <- {x;} in {y;}; (* 1 & 2 & 4 & 5*)
   }
  }; 
};

class A {};
class B inherits A {};
class C inherits B {};
