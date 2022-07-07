# System Programming SIC/XE Assembler 
SIC/XE Assembler implementation  
Project proceed by "System Programming" at Soongsil Univ. 4-1
[Project Review](https://kkanggu39.tistory.com/31)



---
### Assembler 
Refactoring of `Assembler_Former`, implement SIC/XE program into object program using `C`  
Instruction information is at `inst.data`  
SIC/XE program in at `input.text`  
Object program is made into `object_code.obj`  
  
Separate into many functions, so performance got bad  
Trade performance with scability  

Final information is at `g_ObjectCode_table` array  
If wanna build connection with other program and send without file, just add connection and send function  
Or wanna change file format, change only `iPrintObjectCode` function  



---
### GUI Simulator 
Implement GUI simulator using `Java`  
Instruction information is at `inst.data`  
Read object program with `output.obj`  
Former program convert `F1` file to `05` file with adding `EOF`

Structure of this program is not so bad, so I'm not gonna refactoring this simulator  
So output of `Assembler`, `object_code.obj` is different from `output.obj`  



---
### Assembler_Former 
Implement SIC/XE program into object program using `C`  
Instruction information is at `inst.data`  
SIC/XE program in at `input.text`  
Object program is made into `output_20162438` at former version
