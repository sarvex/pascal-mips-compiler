program testFunctionCallMulti;

class testFunctionCallMulti

BEGIN
   
   VAR
      compilerWorks : integer;
      counter	    : integer;


FUNCTION setCompilerWorks(value1: integer; value2: integer ): integer;
BEGIN
   counter := value1 + value2;
   PRINT counter;
   setCompilerWorks := 5
END ;

FUNCTION testFunctionCallMulti;
BEGIN
   compilerWorks := setCompilerWorks(2,8)
END

END
.
