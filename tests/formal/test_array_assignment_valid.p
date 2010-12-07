program testArrayAssignmentValid;

class testArrayAssignmentValid

BEGIN
   
   VAR XRay : ARRAY[0 .. 4] OF integer;
       YRay : ARRAY[8 .. 12] OF integer;

FUNCTION testArrayAssignmentValid;
BEGIN
    xRay[0] := -1;
    xRay[3] := -1;

    yRay[8] := 100;
    yRay[11] := 200;

    XRay := YRay;

    print xray[0]; { prints 100 }
    print xray[3]; { prints 200 }

END

END
.
