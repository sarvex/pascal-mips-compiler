program main;

class room

BEGIN
   VAR doors  : integer;
      windows : integer;
   function room;
   begin
        windows := 3000;
        doors := 4000;
   end
END	      



class main

BEGIN
   
   VAR obj : Room;

FUNCTION main;
BEGIN
    obj := new room;
    print obj.windows;
    print obj.doors;
END

END
.

