program Main;
class Main begin
    function Main;
    begin
    end
    function Test;
        var a1 : Integer;
        var a2 : Integer;
        var a3 : Integer;
        var a4 : Integer;
        var b : Integer;
        var c : Integer;
        var d : Integer;
        var t : Integer;
        var trash1 : Integer;
        var trash2 : Integer;
    begin
        t := b + c;
        if trash1 then begin
            a1 := t + d;
            if trash2 then begin
                a2 := b + c + d;
                t := 0;
                a3 := b + c + d;
            end 
        end else begin
            if trash2 then
                d := b + c
            else
                d := 2 + b
        end;
        a4 := b + c;
        print a4;
    end
end
.

