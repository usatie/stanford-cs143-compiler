class Main {
  main(): Int {42};
  a: Int <- x <- 42; (* undeclared variable y *)
  b: Int <- self <- 42; (* assign to self *) (* conformance check is done after this check *)
  c: Int <- a <- "hello"; (* assign non-conforming expression *)
  d: Int <- x <- y <- z; (* In which order is this error reported? *)
};
