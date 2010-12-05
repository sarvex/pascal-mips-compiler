program Main;
class Main begin
    function Main;
    begin
        doit(true, false, false);
        doit(false, true, false);
        doit(false, false, true);
    end
    function doit( z : Boolean; y : Boolean; x : Boolean);
        var a, b, c : Integer;
    begin
        a := 1;
        a := 1;
        b := 10;
        if z then
            c := 2
        else
            c := 2;
        b := 10;
        while y do begin
            a := 1;
            print a;
            y := false;
        end;
        print b
    end
end
.

