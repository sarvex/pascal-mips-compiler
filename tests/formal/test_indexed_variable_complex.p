program testIndexedVariableComplex;

class room

BEGIN
   VAR doors  : integer;
      windows : integer;
    function room;
    begin
        doors := 100;
        windows := 200;
    end
END	      


class house

BEGIN
   VAR users	 : integer;
      rooms	 : ARRAY[1..2] OF room;
      function house;
      begin
        rooms[1] := new room;
        rooms[2] := new room;
        users := 10;
      end
END		 


class testIndexedVariableComplex

BEGIN
   
   VAR renters : integer;
       my      : house;
       yours   : house;

FUNCTION testIndexedVariableComplex;
BEGIN
    my := new house;
    yours := new house;
   yours.rooms[1].doors := my.rooms[2].doors
END

END
.
