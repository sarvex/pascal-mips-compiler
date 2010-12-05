program Main;
class Main begin
    function Main;
    begin
    end
    function Test;
        var t0 : Integer;
        var t1 : Integer;
        var t2 : Integer;
        var t3 : Integer;
        var t4 : Integer;
        var t5 : Integer;
        var t6 : Integer;
        var a : Integer;
        var b : Integer;
        var c : Integer;
        var d : Integer;
        var trash1 : Boolean;
        var trash2 : Boolean;
    begin
        t6 := a + b;
        t5 := b + c;
        t2 := c + d;
        if trash1 then begin
            d := d + 1;
        end else begin
            t1 := t1 + 1;
        end;
        print 123;
        t3 := b + c; {use value number}
        t4 := c + d; {don't use value number}
        while trash2 do begin
            t0 := a + b; {don't use value number}
            t0 := b + c; {use value number}
            t1 := c + d; {don't use value number}
            while trash1 do begin
                d := d + 1;
                trash1 := false;
                trash2 := false;
            end;
            a := a + 1;
        end;
        t3 := a + b; {don't use value number}
        t3 := b + c; {use value number}
        t3 := c + d; {don't use value number}
    end
end
.

