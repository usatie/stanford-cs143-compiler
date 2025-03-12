-- empty class
class A {};

-- empty inherited class
class B inherits A {};

-- class with single method
class SingleMethod {
    -- method without argument
    no_args(): Int { 42 };
};

-- method
class MultipleMethods {
    -- method with argument
    single_arg(a: Int): Int { 42 };
    -- method with arguments
    multiple_args(a : Int, b : Int, c : Int): Int { 42 };
    -- takes Custom class
    custom_arg(a: SingleMethod): Int { 42 };
};

-- attribute
class Attributes {
    -- attribute
    a : Int;
    -- attribute with initial value
    b : Int <- 42;
};

-- expression
class Expressions {
    assign_expr(): Int { a <- 1 };
    dispatch(s : SingleMethod): Int { s.no_args() };
    dispatch2(m : MultipleMethods): Int { m.single_arg(42) };
    dispatch3(m : MultipleMethods): Int { m.multiple_args(42, 21, 0) };
    single_let(): Int { let a : Int in (42) };
    nested_let(): Int { let a : Int, b : Int, c : Int in (42) };
    plus(a : Int, b : Int): Int { a + b };
    boolean_complement(a : Boolean): Boolean { not a };
    parentheses(): Int { (42) };
    object(a : Int): Int { a };
    int(): Int { 42 };
    str(): String { "Hello, world\n" };
    boolean_true(): Boolean { true };
    boolean_false(): Boolean { false };
};


-- Original good.cl
class AAAAA {
ana(): Int {
(let x:Int <- 1 in 2)+3
};
};

Class BB__ inherits A {
};

