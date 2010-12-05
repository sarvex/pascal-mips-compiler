program Main;
class Main begin
    function Main;
        var a, b, c : Integer;
        var z, y, x : Boolean;
    begin
        print this.otherDude(20, True);
    end

    function otherDude(c : Integer) : integer;
        var b : Boolean;
    begin
        b := c > 10;
        if b then
            otherDude := c
        else
            otherDude := 0;
    end
end
.

