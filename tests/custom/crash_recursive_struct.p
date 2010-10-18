program Main;
class Main begin
    function Main;
        var b1 : B;
    begin
        other(b1);
    end;
    function other(arg : A); beGIN
        print 'evil'
    END
end

class A begin
    var i1: integer;
    var b1: A;
end

class B begin
    var i1: integer;
    var a1: B;
end
.
