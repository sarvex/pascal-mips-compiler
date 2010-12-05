program testIfThenElseNested;

class testIfThenElseNested

BEGIN
FUNCTION testIfThenElseNested;

   VAR aa, bb, cc, dd, ee, ff, xx, yy, zz, c1, c2, c3, c4, c5, c6, ww: integer;

BEGIN   
   aa := 3;
   cc := 23;
   dd := ee + ff;
   
   IF aa = 0 THEN
      bb := 1
   ELSE
   BEGIN
      bb := 0;
      xx := aa + aa + cc + cc;
      zz := cc+cc;
      ww := aa+aa;
      IF cc = 7 THEN
	 
      
         aa := aa
      ELSE
      BEGIN
	 bb := 5;
	 bb := aa + aa + cc
      END;
      yy := aa + cc;
      cc := 4
   END;
   zz := ee + ff;
   cc := ee + ee;
   c1 := ee + 0;
   c2 := ee - 0;
   c3 := ee * 1;
   c4 := ee * 0;
   c5 := ee / 1;
   c6 := 2 * ee

END
   
END   
.

