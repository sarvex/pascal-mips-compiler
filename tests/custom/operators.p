PrOgRaM MAiNE;
clAss maIne begin
    var
        intField1,  intField2  : integer;
        realField1, realField2 : real;
        boolField1, boolField2 : boolean;

    function MainE; begin
        intFIELD1 := 4;
        intField2 := intField1;
        intField1 := 7 * 3 + (intField2 + 2 * intField1 / 2) * 5 + 1;
        realField1 := 1.0;
        realField1 := 2;
        realField2 := intField1;
        realField2 := realField2 / 3;
        realField1 := 1 + 2.0 * 3;
        boolField1 := True;
        boolField2 := not boolField1;
        boolField1 := not boolField1 or (boolField1 and not boolField2);
    end
end
.
