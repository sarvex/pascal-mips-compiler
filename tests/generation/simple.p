program Main;
class Main begin
    function Main;
        var a : Integer;
        var b : Integer;
        var c : Boolean;
        var d : Boolean;
    begin
        a := 1;
        b := 10;
        
        b := b - a;
        print b; { prints 9 }
        
        c := False;
        d := True;
        
        d := c or d;
        print d; { prints True }
    end
end
.

