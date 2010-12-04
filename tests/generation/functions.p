program Main;
class Main
begin
    function Main;
    begin
       printSum(2,8);
    end;

    function printSum(a : integer; b : integer ): integer;
        var c : integer;
    begin
       c := a + b;
       print c;
    end
end
.
