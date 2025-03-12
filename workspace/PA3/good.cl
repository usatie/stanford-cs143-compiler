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
    -- constant
    int_const(): Int { 42 };
    str_const(): String { "Hello, world\n" };
    boolean_const_true(): Boolean { true };
    boolean_const_false(): Boolean { false };
    -- identifier
    idenfier(x : Int): Int { x };
    -- assignment
    assign_expr(): Int { a <- 1 };
    -- dispatch
    dispatch(s : SingleMethod): Int { s.no_args() };
    dispatch2(m : MultipleMethods): Int { m.single_arg(42) };
    dispatch3(m : MultipleMethods): Int { m.multiple_args(42, 21, 0) };
    dispatch_self(): Int { assign_expr() };
    dispatch_self2(): Int { object(42) };
    dispatch_self3(): Int { plus(12, 34) };
    -- conditional
    conditional(a: Int, b: Int, cond: Boolean): Int { if cond then a else b fi };
    -- loop
    simple_loop(): Int { while true loop 42 pool };
    -- case
    single_case(): Int { case e of x: C => 1; esac };
    multiple_cases(): Int { case e of x: C => 1; y: D => 0; esac };
    -- new
    new_(): Int { new Int };
    -- isvoid
    isvoid_(x : Int): Int { isvoid x };
    -- arithmetic op
    parentheses(): Int { (42) };
    plus(a : Int, b : Int): Int { a + b };
    sub(a : Int, b : Int): Int { a - b };
    mul(a : Int, b : Int): Int { a * b };
    divide(a : Int, b : Int): Int { a / b };
    -- comparison op
    lt(a : Int, b : Int): Int { a < b };
    eq(a : Int, b : Int): Int { a = b };
    leq(a : Int, b : Int): Int { a <= b };
    -- unary op
    integer_complement(x : Int): Int { ~x };
    boolean_complement(a : Boolean): Boolean { not a };
    -- let
    single_let(): Int { let a : Int in (42) };
    nested_let(): Int { let a : Int, b : Int, c : Int in (42) };
};

class MoreExpressions inherits Expressions {
    static_dispatch_self(): Int { self@Expressions.assign_expr() };
    static_dispatch_self2(): Int { self@Expressions.object(42) };
    static_dispatch_self3(): Int { self@Expressions.plus(12, 34) };
};


-- Original good.cl
class AAAAA {
ana(): Int {
(let x:Int <- 1 in 2)+3
};
};

Class BB__ inherits A {
};

