# Phase 1: Echo Client-Server

This project is focused on implementing a client-server communication system using sockets in the C programming language. The main objective is to create an "Echo Client-Server" pair, where the server echoes the message received from the client.

The first part of the assignment involves creating a client and a server program that can communicate with each other via sockets. The server program should echo (repeat back) the message received from the client.

To simplify the implementation, the following assumptions are made:

    The message to the server and the response will not exceed 15 bytes.
    Memory allocation can be done statically (e.g., char buffer[16];).
    Each send or recv call will handle the full message.
    The response from the server is not required to be null-terminated.

The client program supports the following command-line options:

```
usage:
echoclient [options]
options:
-s                  Server (Default: localhost)
-p                  Port (Default: 8080)
-m                  Message to send to server (Default: "Hello World!")
-h                  Show this help message
```

The server program's command-line interface only needs to allow the port to be specified:

```
usage:
echoserver [options]
options:
-p                  Port (Default: 8080)
-n                  Maximum pending connections
-h                  Show this help message
```

The server should not terminate after sending its first response but should be prepared to serve another request.

Please refer to the provided starter code and the suggested references for further guidance.

# Project Structure
```
echo/
  ├── echoclient.c
  ├── echoserver.c
  └── readme.md
```
The echo/ directory contains the following files:
```
echoclient.c: The client program that sends a message to the server.
echoserver.c: The server program that echoes the received message.
readme.md: A template file for taking project notes and documenting the approach.
```
# Resources
This project utilized the following resources in order to complete it:
  
* Sockets API documentation: [C Socket Programming](https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html)
* Beej's Guide to Network Programming: [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
* Online tutorials and examples on socket programming in C.
