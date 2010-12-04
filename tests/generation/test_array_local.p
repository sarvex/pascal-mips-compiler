program testArrayLocal;

class testArrayLocal

BEGIN
   
VAR
   global  : ARRAY[0..99] OF integer; 
   counter : integer;
FUNCTION testArrayLocal;
VAR local  : ARRAY[0..9] OF integer;
BEGIN
   local[5] := 88;
   PRINT local[5];
   
   counter := 0;
   WHILE counter <= 50 DO
   BEGIN
      global[counter] := counter;
      PRINT global[counter];
      counter := counter + 1;
      print global[counter];
      PRINT counter
   END;

   PRINT local[5]
END

END
.
