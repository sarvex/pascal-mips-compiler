program Main;
class Main begin
    function Main;
    begin
        doit(1, false)
    end
    function doit( a : Integer; c : Boolean );
        var b : Integer;
        var d : Boolean;
    begin
        b := a + 0;
        b := a * 1;
        b := a - 0;
        b := a * 0;
        b := a - a;
        b := a / 1;
        b := 2 * a;
        b := a / a;
        b := a mod a;
        b := 0 mod a;
        b := 0 / a;
        b := 0 - a;
        print b;

        d := c and c;
        d := c and false;
        d := c or c;
        d := c or true;

        print b;

        d := not c;
        print d;

        
        { unsupported 
        a >> 0 = a
        a << 0 = a
        a ** 1 = a
        a ** 2 = a * a
        max(a,a) = a
        min(a,a) = a }
    end
end
.

