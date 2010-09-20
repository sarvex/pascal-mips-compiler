program Main;
class Main begin
    function Main;
        var a1 : A;
        var b1 : B;
    begin
        a1 := new A;
        a1.f1;
        print a1.a1;
        b1 := new B;
        b1.f1;
        print b1.a1;
    end
end

class A extends B begin
    var a1 : boolean;
    function f1;
        var b1 : boolean;
    begin
        b1 := false;
    end
end

class B begin
    var a1 : integer;
    function f1; begin
        a1 := 64 / 8 * 2 mod 10;
    end
end
.


