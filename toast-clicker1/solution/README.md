# Toast Clicker 1
Flag is in the Java code,  in `Main Activity`. It is created in a function that is never invoked. 
```
int input[] = {67, 83, 68, 120, 62, 109, 95, 90, 92, 112, 85, 73, 99, 82, 53, 99, 101, 92, 80, 89, 81, 104};
String printfirstFlag(){
        String output = "";
        for(int i = 0; i < input.length; i++ ){
            int t = input[i] + i;
            output += Character.toString((char) t);
        }
        return output;
    }
```

Basically it is ascii (array element + position in array). 

```>>> input
[67, 83, 68, 120, 62, 109, 95, 90, 92, 112, 85, 73, 99, 82, 53, 99, 101, 92, 80, 89, 81, 104]
>>> result = map(lambda (i,e): chr(i+e), enumerate(input))
>>> print(''.join(result))
CTF{Bready_To_Crumble}
```