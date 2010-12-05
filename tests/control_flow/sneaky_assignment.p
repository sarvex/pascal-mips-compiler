program Main;
class Main begin
    function  Main;
    begin
        doit(true);
        doit(false)
    end
    function doit( c : Boolean);
        var a : Integer;
        var b : Integer;
    begin
        if c then begin
            a := 1;
            b := 4;
        end else begin
            a := 2;
            b := 3;
        end;
        print a + b;
    end
end
.

