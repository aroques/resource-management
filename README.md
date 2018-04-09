## Resource Management

**OSS (operating system similator)** is a program that simulates an operating system.

Resources are managed by OSS. User processes request resources. OSS will then check if that resources is available. If the resource is available, then OSS will run the Banker's Algorithm to check if there is a safe sequence after the resource is granted. If there is a safe sequence, then OSS will grant the resource. A *safe sequence* is an ordering of processes. For example, <P2, P4, P0, P1, P3> could be a safe sequence and that would mean that P2 could finish executing and then would release its resources. Then P4 would finish executing and release its resources, and so on until all the processes have finished executing and all the resources have been released. Thus, a safe sequence gurantees that there cannot be a deadlock.

### Program Hanging
The program has a low probability to stall for a reason that I have not been able to figure out. If that happens, then press ctrl-c to exit the program and try running it again.

The program will generate a log file called oss.log. Use the -v (verbose) flag to have more information printed to the console and log file.

To build this program run:
```
make
```

To run this program:
```    
./oss
```

To run this program in verbose mode:
```    
./oss -v
```

To cleanup run:
```
make clean
```

#### Below is my git log: