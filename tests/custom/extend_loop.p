program Main;
class Main begin
    function Main;
        var a1 : A;
        var b1 : B;
    begin
        a1 := new A;
        b1 := new B;
    end
end

class A extends B begin
    function f;
        var b1 : B;
    begin
        g;
        this.g;
        b1 := new B;
        b1.h;
    end
    function g;
        var m : Main;
    begin
        m := m;
    end
end

class B extends C begin
    var a1 : integer;
    function h; begin
        a1 := 23;
    end
end

class C extends A begin
    var z : boolean;
    function rawr; begin
        z := true;
    end
end
.


