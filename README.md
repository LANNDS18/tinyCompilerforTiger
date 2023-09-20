# Tiger Compiler


## Run

### Build Tiger Compiler

```
mkdir build
cd build
cmake ..
cmake --build.
cd ..
```

### Compiler Executable .tig file

```
make clean
make SRC_FILE=<testcases/test.tig> OUTPUT=<test>
```

### Run the compiled Tiger program

```
./test.tigex
```

## Tiger Definition

### Lexical Elements

#### Keywords:
- while, for, to, break, let, in, end, function, var, type, array, if, then, else, do, of, nil

#### Special Symbols:
- , : ; + - * / ( ) [ ] { } = < > & | .

#### Operators:
- =, <>, <, <=, >, >=, &, |, :=

### Declaration

#### Type Declaration
- Name Type
- Record Type
  - Example:
      ```
      let
          type student = {name: string, age: int, isBoss: bool}
      in
          var lannds: student := student{name = "LANNDS Z", age = 21, isBoss = false}
      end
      ```
- Array Type
  - Example:
    ```
    let
        type intArray = array of int
    in
        var arr: intArray := intArray [10] of 0
    end
    ```
  
#### Variable Declaration
- Simple Variable (`A_SimpleVar`)
- Field Variable (`A_FieldVar`) :  A reference to a field within a record type
- Subscript Variable (`A_SubscriptVar`): An element within an array

#### Function Declaration
- Function (`A_functionDec`)
  - Example:
    ```
    function initMatrix(mat : matrix, row : int, col : int) =
        for i := 0 to row-1 do
        mat[i] := vector[col] of 0
    ```


### Expression

#### Arithmetic and Comparison Operators
- Addition: `+`
- Subtraction: `-`
- Multiplication: `*`
- Division: `/`
- Equal to: `=`
- Not equal to: `<>`
- Less than: `<`
- Less than or equal to: `<=`
- Greater than: `>`
- Greater than or equal to: `>=`

#### Other Expressions
- Variable Expression (`A_varExp`): `:=`
- Nil Expression  (`A_nilExp`): 
``` 
type person = {name: string, age: int} 
var unknownPerson: person := nil
```
- Integer Literal (`A_intExp`): `10`, `-1`, `0`, ...
- String Literal (`A_stringExp`):
```
let
  var hello: string := "Hello, World!" 
in
  ...
end
```
- Function Call (`A_callExp`)


