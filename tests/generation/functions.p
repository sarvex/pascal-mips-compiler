program Main;
class Main
begin
    function Main;
    begin
       print addThese(2,8);
    end;

    function addThese(a : integer; b : integer ) : integer;
        var c : integer;
    begin
       addThese := a + b;
    end
end
.
