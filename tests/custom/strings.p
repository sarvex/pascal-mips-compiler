program strings;
class strings begin
    var hello : String;
    var world : String;
    var space : String;
    var bang : String;
    var helloWorld : String;

    function strings; begin
        hello := s('H', s('e', s('l', s('l', s('o', nil)))));
        space := s(' ', nil);
        world := s('W', s('o', s('r', s('l', s('d', nil)))));
        bang := s('!', nil);
        helloWorld := hello.cat(space.cat(world.cat(bang)));
        helloWorld.printString();
    end;
    function s(c : char; n : String) : String; begin
        s := new String(c, n);
    end
end
class String begin
    var c : char;
    var next : String;
    function String(ch : char; nxt : String); begin
        c := ch;
        next := nxt
    end;
    function cat(s : String) : String; begin
        if next = nil then
            cat := new String(c, s)
        else
            cat := new String(c, next.cat(s));
    end;
    function printString; begin
        print c,;
        if next <> nil then
            next.printString();
    end
end

.

