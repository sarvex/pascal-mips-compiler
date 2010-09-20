program testFunctionCallObject;

class testFunctionCallObject BEGIN
    VAR retval  : integer;
    VAR counter : REAL;

    FUNCTION test(value1 : integer; value2: REAL): integer; BEGIN
       counter := value2;
       test := 3
    END;

    FUNCTION testFunctionCallObject; BEGIN
       retval := test(4, retval)
    END
END
.
