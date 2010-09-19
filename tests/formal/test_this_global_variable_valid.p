program testThisGlobalVariableValid;


class testThisGlobalVariableValid

BEGIN
   VAR
      notThis : integer;


FUNCTION testThisGlobalVariableValid;
VAR someVar : integer;
BEGIN
   this.notThis := 6;
   notThis := 5;

   PRINT this.notThis
END

END
.
