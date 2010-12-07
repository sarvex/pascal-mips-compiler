program lists;
class lists begin
    var first : List;
    var third : List;
    var second : List;
    var allOfThem : List;
    var nil : List;

    function lists; begin
        first := s(0, s(1, s(2, s(3, s(4, nil)))));
        second := s(5, nil);
        third := s(6, s(7, s(8, s(9, s(10, nil)))));
        allOfThem := first.cat(second.cat(third));
        allOfThem.printList();
    end;
    function s(data : integer; n : List) : List; begin
        s := new List(data, n);
    end
end
class List begin
    var data : integer;
    var next : List;
    var nil : List;
    function List(data2 : integer; next2 : List); begin
        data := data2;
        next := next2;
    end;
    function cat(s : List) : List; begin
        if next = nil then
            cat := new List(data, s)
        else
            cat := new List(data, next.cat(s));
    end;
    function printList; begin
        print data;
        if next <> nil then
            next.printList();
    end
end

.

