program Main;
class Main begin
    function Main;
        var a1 : A;
        var b1 : B;
    begin
        a1 := new A;
        a1.f;
        b1 := new B;
        b1.g;
    end
end

class A begin
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

class B begin
    var a1 : A;
    function h; begin
        a1 := new A;
    end
end
.

