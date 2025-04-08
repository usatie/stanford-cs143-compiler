class Main {
  b: Bool;
  i: Int;
  s: String;
  x: X;
  y: Y;
  v: Void;
  main(): Object {
    {
      ~b; (* Illegal *)
      ~i;
      ~s; (* Illegal *)
      ~x; (* Illegal *)
      not b;
      not i; (* Illegal *)
      not s; (* Illegal *)
      not x; (* Illegal *)

      (* Addition *)
      b + b; (* Illegal *)
      b + i; (* Illegal *)
      s + s; (* Illegal *)
      s + i; (* Illegal *)
      x + x; (* Illegal *)
      x + i; (* Illegal *)
      i + i;

      (* Subtraction *)
      b - b; (* Illegal *)
      b - i; (* Illegal *)
      s - s; (* Illegal *)
      s - i; (* Illegal *)
      x - x; (* Illegal *)
      x - i; (* Illegal *)
      i - i;

      (* Multiplication *)
      b * b; (* Illegal *)
      b * i; (* Illegal *)
      s * s; (* Illegal *)
      s * i; (* Illegal *)
      x * x; (* Illegal *)
      x * i; (* Illegal *)
      i * i;

      (* Division *)
      b / b; (* Illegal *)
      b / i; (* Illegal *)
      s / s; (* Illegal *)
      s / i; (* Illegal *)
      x / x; (* Illegal *)
      x / i; (* Illegal *)
      i / i;

      (* Less than *)
      b < b; (* Illegal *)
      b < i; (* Illegal *)
      s < s; (* Illegal *)
      s < i; (* Illegal *)
      x < x; (* Illegal *)
      x < i; (* Illegal *)
      i < i;

      (* Less than or equal *)
      b <= b; (* Illegal *)
      b <= i; (* Illegal *)
      s <= s; (* Illegal *)
      s <= i; (* Illegal *)
      x <= x; (* Illegal *)
      x <= i; (* Illegal *)
      i <= i;

      (* Equal to *)
      b = b;
      i = i;
      s = s;
      b = i; (* Illegal *)
      b = x; (* Illegal *)
      s = x; (* Illegal *)
      i = x; (* Illegal *)
      x = x;
      y = y;
      x = y; (* This is OK, because On non-basic objects, equality simply checks for pointer equality *)
      x = (new Object); (* This is also OK *)
      (new IO) = x; (* This is OK, because IO is a subclass of Object *)
    }
  };
};

class X {};
class Y {};
