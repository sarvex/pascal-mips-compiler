program testFunctionCallObject;

class testFunctionCallObject BEGIN
    VAR retval  : integer;
    VAR counter : integer;

    FUNCTION test(value1 : integer; value2: integer): integer; BEGIN
       counter := value2;
       test := 3
    END;

    FUNCTION testFunctionCallObject; BEGIN
       retval := test(4, 5, 6, 7, 8)
    END
END
.
