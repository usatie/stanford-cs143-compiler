class Main inherits IO{
 x: Int;
 main(): Object {{ 
		   if ((let x:Int <- 5 in x+3)+3 = 9) then 3 else foo() fi;
                }};
 foo(): String {"test"};
 bad_pred_type(): Int { if (new Main) then 3 else 4 fi };
};
