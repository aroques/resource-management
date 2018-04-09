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
0703d7b 2018-04-09 17:18:31 -0500 Add more information to README  
a68c369 2018-04-08 08:15:47 -0500 Add verbose mode  
63d8ba2 2018-04-07 20:33:40 -0500 Add total wait time to statistics  
a66ec98 2018-04-07 20:02:04 -0500 Add statistics  
4710dd7 2018-04-07 19:26:38 -0500 Test/debug program  
d535610 2018-04-06 19:55:36 -0500 Check if we can unblock more than 1 process per resource when process terminate  
3088948 2018-04-06 19:41:03 -0500 Test blocked queue  
89f8b5f 2018-04-06 18:07:30 -0500 Implment blocked queue  
bcc0e31 2018-04-06 17:33:36 -0500 Add print rsc summary function and blocked queue  
d9b5e5a 2018-04-06 16:43:23 -0500 Cleanup OSS  
44f87e2 2018-04-06 16:08:33 -0500 Change message receive function  
354edbb 2018-04-01 08:04:33 -0500 Cleanup OSS main loop  
eef4ce0 2018-04-01 07:48:36 -0500 Debug and move resource functions from bankers file into resources file  
02e84b5 2018-03-31 19:44:20 -0500 Refactor rsc table print functions to also print to file  
b752d4c 2018-03-31 19:27:43 -0500 Move resource functions from user program into resource file  
33ed701 2018-03-31 15:40:37 -0500 Add logic so user only requests a resource if allocated is less than max claims  
603b7a0 2018-03-31 13:56:55 -0500 Add TBDs for more OSS logic  
0049183 2018-03-31 13:33:32 -0500 Move get max rsc claims function into resource file  
e252a0e 2018-03-31 13:29:03 -0500 Refactor resources functions from OSS into resources file  
313a087 2018-03-31 13:04:21 -0500 Finish bankers algorithm  
ad9d7ba 2018-03-31 12:45:33 -0500 Fix build errors  
dba55e3 2018-03-31 12:36:19 -0500 Refactor bankers algorithm into its own file  
39ee351 2018-03-30 19:20:43 -0500 Add bankers algorithm logic  
cce829d 2018-03-30 18:34:21 -0500 Add bankers algorithm  
640e48c 2018-03-30 15:28:48 -0500 Lower request/release time for user program  
6187f08 2018-03-30 14:38:52 -0500 Add message parsing logic to OSS  
2b0b0a2 2018-03-29 20:00:33 -0500 Add rcv msg no wait function  
2a8e00e 2018-03-29 19:38:13 -0500 Add get available pid function  
c851a5a 2018-03-29 19:17:51 -0500 Refactor msg coordination, add TBD comments  
d706f46 2018-03-29 18:29:44 -0500 Get number of allocated resources function  
c8a2fc2 2018-03-29 17:17:30 -0500 Fix errors and warning messages in user program  
e776dbf 2018-03-29 17:06:53 -0500 Refactor request/release logic into functions  
b6576d6 2018-03-29 16:53:07 -0500 Add release resource logic to user program  
1897fd7 2018-03-29 14:24:21 -0500 Refactor user program  
6902dbe 2018-03-29 14:15:19 -0500 Cleanup user program  
ed6e008 2018-03-29 13:52:51 -0500 Add request rsc logic to user  
95a5e7b 2018-03-29 13:25:30 -0500 Fix bug in print allocated resource table  
3a3a4af 2018-03-29 13:20:23 -0500 Add print allocated resource table function  
f01882a 2018-03-27 21:22:16 -0500 Add allocated array and stub user program  
181a7c4 2018-03-27 07:11:06 -0500 Cleanup user program  
1d78e0b 2018-03-27 06:57:45 -0500 Cleanup user program  
794f2b8 2018-03-27 06:15:40 -0500 Add allocation of resource table  
ac65136 2018-03-27 06:06:44 -0500 Update user for new shared memory structures  
e58e5f6 2018-03-27 06:01:50 -0500 Add shared memory structures  
8b21d62 2018-03-25 20:08:35 -0500 First commit  