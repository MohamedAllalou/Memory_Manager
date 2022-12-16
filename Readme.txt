OVERVIEW:
The main objective of this C++ program is to simulate an operating system’s virtual memory management and concurrency control. The virtual memory being managed consists of a main memory and a large disk space. Both are divided into ‘pages’. The number of pages of the main memory is limited while disk space has unlimited number of pages. Each page stores a variable in the form (id, value), such that id is the identifier of the variable and value is the value associated with the variable. To manage (store, search, and remove) variables, the virtual memory manager offers three APIs to the processes:
1) Store (string variableId, unsigned int value): This instruction stores the given variable id and its value in the first unassigned spot in the memory.
2) Release (string variableId): This instruction removes the variable id and its value from the memory so the page which was holding this variable becomes available for further storage.
3) Lookup (string variableId): This instruction checks if the given variableId is stored in the memory and returns its value or -1 if it does not exist. If the Id exists in the main memory it returns its value. If the Id is not in the main memory but exists in disk space (i.e. page fault occurs), then it should move this variable into the memory and release the assigned page in the virtual memory. Note that if no spot is available in the memory, the program needs to swap this variable with the least recently accessed variable, i.e. the variable with smallest Last Access time, in the main memory.

IMPLEMENTATION:
Input: Input will consist of 3 files. Firstly, a “memconfig.txt” file which contains the number of pages in the main memory. 
Secondly, a “processes.txt” file. The first line contains the number of cores C. The second line contains the number of processes N. Then next N number of lines each contain the process start time followed by its duration.
Lastly, a “commands.txt” file. Each line contains a command to be executed by the processes. Any process should continually pick one command from the list of commands. The command that will be picked at any time is the next command (the one after the command last picked by any process). If all the commands have been executed, the process should simply start over from the beginning and begin picking commands for a second time, then a third time, etc…. To invoke the commands the processes should call the suitable API of the virtual memory manager based on the picked command. The process should wait for a random time (from 1 to 1000 milliseconds) between each API call to simulate the time each API call may take. 
Following are a list of sample input/output files.
1) “memconfig.txt”
-----
2
-----

2) “processes.txt”
-----
2
3
2 3
1 2
4 3
-----

3) “commands.txt”
-----
Store 1 5
Store 2 3
Store 3 7
Lookup 3
Lookup 2
Release 1
Store 1 8
Lookup 1
-----


Output: The program should output a file called “output.txt” that captures the following events with their associated information:
1) The time each process has started/finished, with the value of the clock at that time, the id of the started process, and the name of the event (Started or Finished).
2) Each command executed by a process. It should include the value of the clock at the time the command was invoked, the name of the command, its arguments, and its output if any (like in the case of lookup; see the example below).
3) Other events of the memory manager: Mainly the swapping when needed within a lookup. The string tracing this event should also contain the Ids of the variables being swapped, the first variableId being the variable id of the variable that was in the disk space, and the second being the variableId of the variable that was in the main memory.
Below is an example of the output file:

“output.txt”
-----
Clock: 1000, Process 2: Started.
Clock: 1010, Process 2, Store: Variable 1, Value: 5
Clock: 1730, Process 2, Store: Variable 2, Value: 3
Clock: 2000, Process 1: Started.
Clock: 2010, Process 1, Store: Variable 3, Value: 7
Clock: 3000, Process 2: Finished.
Clock: 3020, Memory Manager, SWAP: Variable 3 with Variable 1
Clock: 3030, Process 1, Lookup: Variable 3, Value: 7
Clock: 3100, Process 1, Lookup: Variable 2, Value: 3
Clock: 3800, Process 1, Release: Variable 1
Clock: 4000, Process 3: Started
Clock: 4200, Process 3, Store: Variable 1, Value 8
Clock: 4400, Memory Manager, SWAP: Variable 1 with Variable 3
Clock: 4410, Process 1, Lookup: Variable 1, Value 8
Clock: 5000, Process 1: Finished.
Clock: 7000, Process 3: Finished.
-----

Implementation Rules
1) Processes should be simulated by threads. The algorithm used to schedule the processes is First In, First Out. The maximum number of processes that can run at the same time equals the number of cores given as input in processes.txt.
2) The First in First out scheduler should be running on its own thread.
3) The program clock should be running on its own thread.
4) Virtual memory: the pages in the main memory should be simulated by an array in the actual computer physical memory, while disk pages are simulated by an array located into a text file such as “vm.txt”, which must be accessed every time we need to access the disk.
5) In order to protect critical sections and ensure mutual exclusion, use appropriate tools (for example; semaphores), within the body of each function.
