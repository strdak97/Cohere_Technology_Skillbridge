# Phase 2: Transfer Lab

In this phase of the project, the focus is on transferring files between the client and the server. It is important to note that when using the send function, the operating system does not guarantee that the entire memory range specified will be copied to the socket. Instead, it may copy as much memory as it can fit in the network buffer and returns the amount copied via the return value.

For example, after the send call:
```
int socket;
char *buffer;
size_t length;
ssize_t sent;

/* Preparing message */
...

/* Sending message */
sent = send(socket, buffer, length, 0);

assert(sent == length); // WRONG!
```
It is incorrect to assume that sent will always be equal to length. The send function may need to be called again with different parameters to ensure that the entire message is sent.

Similarly, for the recv function, when receiving data from a TCP socket, the operating system does not wait for the entire memory range specified to be filled with data before returning. Instead, it may only deliver a portion of the original data sent. The amount of data written to the specified memory address is returned by the recv call.

This behavior is desirable in various scenarios, both from the perspective of resource sharing by the operating system and the user's perspective. For example, if the goal is to save received data to disk, it can be done chunk by chunk without storing the entire file in memory.

For the second part of the warm-up, the objective is to create a client program that connects to a server and saves the received data to a file on disk.

The client program should support the following command-line options:
```
usage:
transferclient [options]
options:
-s                  Server (Default: localhost)
-p                  Port (Default: 8080)
-o                  Output filename (Default: foo.txt)
-h                  Show this help message
```
The client connects to the server and receives data, which is then saved to the specified output filename. Make sure to consider file permissions and provide read and write access (e.g., S_IRUSR | S_IWUSR) when opening the file.

The server program begins transferring data from a file to the client over the network once a connection is established. It closes the socket when the transfer is complete.

The server program should support the following command-line options:
```
usage:
transferserver [options]
options:
-p                  Port (Default: 8080)
-f                  File to serve up (Default: bar.txt)
-h                  Show this help message
```
The server listens on the specified port and transfers the data from the specified file to the client.

You may consult the suggested references in this document or other resources available online for further guidance.

# Project Structure
```
transfer/
  ├── transferclient.c
  ├── transferserver.c
  └── readme.md
```

The transfer/ directory contains the following files:
```
transferclient.c: The client program that receives data from the server and saves it to a file.
transferserver.c: The server program that transfers data from a file to the client.
readme.md: A template file for taking project notes and documenting the approach.
```

# Resources
This project utilized the following resources in order to complete it:
  
* Sockets API documentation: [C Socket Programming](https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html)
* Beej's Guide to Network Programming: [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
* Online tutorials and examples on socket programming in C.
