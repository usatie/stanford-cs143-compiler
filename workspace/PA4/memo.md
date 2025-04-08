# self and SELF_TYPE
- `self` has `SELF_TYPE`
## Allowed uses of `SELF_TYPE`
- new                 : `new SELF_TYPE`
- method return type  : `f(): SELF_TYPE`
- let type decl       : `let x: SELF_TYPE`
- attribute type decl : `x: SELF_TYPE`
## Illegal uses of `SELF_TYPE`
- Case branch         : `case x of y: SELF_TYPE => ...`
- Formal parameter    : `f(x: SELF_TYPE)`
- Static dispatch     : `x@SELF_TYPE.f(...)`
- Class Declaration   : `class SELF_TYPE { ... }`
## Meaning of `SELF_TYPE`
- `SELF_TYPE` depends on the class in which it is used.
