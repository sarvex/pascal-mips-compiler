program testArrayMultiBoundsValid;

class testArrayMultiBoundsValid begin
    var big : array[0..2] of array[0..2] of Thing;
    function testArrayMultiBoundsValid; begin
        big[0,1] := new Thing(44);
        big[1,1] := new Thing(33);
        big[0] := big[1];
        print big[0,1].data;
    end

end

class Thing begin
    var data : integer;
    function Thing(var data2 : integer); begin
        data := data2;
    end
end
.

