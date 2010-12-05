program Main;
class Main begin
    function Main;
    begin
        doit(1, 2, 3);
    end
    function doit( b : Integer; c : Integer; d : Integer);
        var a1 : Integer;
        var a2 : Integer;
        var a3 : Integer;
        var t : Integer;
    begin
        t := b + c;
        a1 := t + d;
        a2 := b + c + d;
        t := 0;
        a3 := b + c + d;
        print a1;
        print a2;
        print a3;
        print t;
    end
end
.

