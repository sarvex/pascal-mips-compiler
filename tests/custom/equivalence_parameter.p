program Main;
class Main begin
    function Main;
        var a1 : A;
        var b1 : B;
        var c1 : C;
        var d1 : D;
    begin
        other(b1, d1);
    end
    function other(arg : A; arg2 : C); beGIN
        print 'evil'
    END
end

class A begin
    var a : integer;
    var b : boolean;
    var c : C;
    var d : array[0..9] of boolean;
    var e : char;
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
    var a : char;
    var b : boolean;
    var c : D;
    var d : array[1..10] of boolean;
    var e : char;
    function h; begin
        print 'booga'
    end
end

class C begin
    var a : real;
    var b : integer;
    var c : array[2..4] of real;
end

class D begin
    var a : integer;
    var b : char;
    var c : array[0..2] of integer;
end
.

