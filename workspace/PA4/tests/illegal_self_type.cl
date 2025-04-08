class Main { main(): Int {42}; };

class A {
  a: SELF_TYPE;
  (*
  self_type_case_branch: Int <- case 42 of
      x: SELF_TYPE => x;
      x: Int => new B;
    esac; (* Inferred is A, it means even though the SELF_TYPE use is illegal in case branch, the branch is used as type inferrence. *)
  self_type_case_branch_dup: Int <- case 42 of
      x: SELF_TYPE => x;
      x: SELF_TYPE => new A; (* dup branch with SELF_TYPE *)
      x: Int => new B;
    esac; (* Inferred is A, it means even though the SELF_TYPE use is illegal in case branch, the branch is used as type inferrence. *)
  self_case_branch_A: Int <- case 42 of
      self: A => self;
      x: Int => new B;
    esac;        (* Inferred is A, it means even though the self-bound is illegal in case branch, the branch is used as type inferrence. *)
  self_case_branch_B: Int <- case 42 of
      self: B => self;
      x: Int => new B;
    esac;        (* Inferred is B, it means even though the self-bound is illegal in case branch, the branch is used as type inferrence. *)
  *)
  self_type_case_branch_with_self: Int <- case 42 of self: SELF_TYPE => 42; x: Int => 42; esac;
  self_type_formal(x: SELF_TYPE): Int { 42 };
  self_type_static_dispatch: Object <- (new A)@SELF_TYPE.g();
};

class B inherits A {};
