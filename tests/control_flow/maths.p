program Main;
class Main begin
    function Main;
        var a : Integer;
        var b : Integer;
        var c : Boolean;
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

        d := c and c;
        d := c and false;
        d := c or c;
        d := c or true;
        
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

