program testFunctionCallObjectExtend;

class myCustomClass

BEGIN
   
   VAR
      lightSwitch	      : boolean;
      compilerWorks	      : boolean;

FUNCTION setCompilerWorks(value	: boolean ): integer;
BEGIN
   compilerWorks := value;
   setCompilerWorks := 2
END   
   
END


class middleManClass extends myCustomClass

BEGIN
END   


class testFunctionCallObjectExtend

BEGIN
   
   VAR
      onOff	     : boolean;
      myCustomObject : middleManClass;
      retval	     : integer;
   
FUNCTION testFunctionCallObjectExtend;
BEGIN
    myCustomObject := new middleManClass;
    retval := myCustomObject.setCompilerWorks(onOff);
    print retval;
END

END
.
