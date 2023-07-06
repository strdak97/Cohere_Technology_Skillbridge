# Cohere Technology Skillbridge

**FOUND-110**

Discusses and reviews the fundamentals of software development and exploitation. Utilizes reading materials such as the Art of Exploitation along with Labs and CTF challenges. 

- Learn the fundamentals of C coding and x86 Assembly
- How to debug code with GBD
- How to read low-level process layout in memory
- Basic exploitation concepts:
  * Stack and Heap Buffer Overflows
  * Overwriting Function Pointers
  * Format String vulnerabilities 
- Basic networking concepts:
  * Programming C sockets
  * The TCP/IP Stack
- Writing basic shellcode:
  * Linux bind shell
  * Linux reverse shell

**FUND-120**

Builds on the concepts learned in FOUND-110 utilizing Carnegie Mellon University's Introduction to Computer Systems [course](https://scs.hosted.panopto.com/Panopto/Pages/Sessions/List.aspx#folderID=%22b96d90ae-9871-4fae-91e2-b1627b43e25e%22&view=0&maxResults=50). This unit has five labs that must be completed:

- C Programming Lab
  * Provides an overview of C Programming and has the student create a queue that supports both last in, first out (LIFO) and first-in, first-out FIFO) principals. This lab tests the following: 
    + Explicit memory management within the C programming language
    + Creating and manipulating pointer-based data structures
    + Working and manipulating strings
    + Utilizing data structures such as functions or arrays to store redundant data
    + Implmenting robust and flexible C code that properly handles invalid arguemnts or NULL pointers

- Data Lab
  * Has the student become more familiar with bit-level representations of integers and floating point numbers by solving a series of programming "puzzles".
  * With 15 different puzzles, the student uses 8 binary operators (! ~ & | + << >>) to manipulate various data types and pass a series of unit tests.

- Bomb Lab
  * The bomb lab helps teach students foundational principles in machine-level programming, as well as general debugging and reverse engineering skills.
  * Utilizing GDB the student debugs a "binary bomb" which contains six phases which all require a secret key in order to defuse the "bomb".
    + In order to defuse the bomb students must utilize a debugger to single-step through the program at each phase to determine what each assembly statement is doing to determine the string needed to defuse that stage of the bomb. 
    + Points are earned after each phase is defused and everytime the bomb explodes points are lost, utilizing breakpoints and backtraces are vital in order to pass the lab.
 
- Attack Lab
  * Requires student to develop and implement five different attacks on two programs with varying security vulnerabilites. This lab will allow students to:
    + Research different ways attackers can exploit security vulnerabilities when programs do not properly safeguard themselves from buffer overflows.
    + Understand different security techniques (Stack canary's, ROP, ASLR) that have been implemented to help prevent programs from being vulnerable to buffer overflows.
    + Obtain a in-depth understanding of stack and parameter-passing mechanisms of x86-64 assembly language.
    + Garner extensive experience utilizing debugging tools such as GDB and OBJDUMP within UNIX operating systems.
   
- Client-Server Lab
  * This is a four-part lab in which the ultimate goal is to have students design and implement a multi-threaded web server as well as a multi-threaded client that acts as a load generator for the server.
    + The first phase of the lab starts by having students implement a simple echo server and client, in which the server simply echoes (repeats back) the message that the client sends.
    + The second phase of this lab expands on the echo lab by having the student implement a transfer server and lab that takes a single file request from the client and has the server provide it to be saved on disk. 
    + The third phase of the lab has student practice creating library functions by having them create a custom library called GETFILE to add to their transfer server. 
    + The fourth and final phase of the lab has student take everything learned up until this point and modify the GETFILE program to become utilize multi-threading capabilities along with mutex locks and conditionals.

**TRACK-201**
  
