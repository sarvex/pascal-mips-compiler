program testArrayAssignmentValid;

class testArrayAssignmentValid

BEGIN
   
   VAR XRay : ARRAY[0 .. 4] OF ARRAY[0 .. 4] of integer;
       YRay : ARRAY[8 .. 12] OF ARRAY[0 .. 4] of integer;

FUNCTION testArrayAssignmentValid;
BEGIN
    xRay[0][1] := -1;
    xRay[0][2] := -1;

    yRay[8][1] := 100;
    yRay[8][2] := 200;

    XRay[0] := YRay[8];

    print xray[0, 1]; { prints 100 }
    print xray[0, 2]; { prints 200 }

END

END
.

