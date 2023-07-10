# Phase 3: GETFILE Protocol
Phase 3 of the Client Server lab has the student implement a GETFILE protocol which is a simple protocol based off of [libcurl’s
“easy” interface](http://curl.haxx.se/libcurl/c/libcurl-easy.html). This phase is divided into two different sections the first section being the GETFILE Client,
and the second section being the GETFILE Server. 

The GETFILE Client provides an interface to the server in order to operate as a workload generator. 
The program implements callback functions in order to handle headers and body chunks for the response. 
The student is responsible for creating a series of library functions that will be implemented by the client,
in order to handle the callback functions as well as a sortment of other features. 

The GETFILE Server is inspired by Python's built-in HTTP Server. 
The gfserver.h header file defines the server-side API, and the gfserver.c file contains the implementation of the functions. 
The handler.c file provides an example of how to use the server library by registering a handler callback function to handle requests. 
The callback should utilize the provided functions, such as gfs_sendheader and gfs_send, to send the response to the client.
The student is tasked with creating the GETFILE Server library that provide a variety of features to the server. 

# Project Structure
```
gflib/
  ├── gfclient.c
  ├── gfclient.h
  ├── gfclient_download.c
  ├── gfserver.c
  ├── gfserver.h
  ├── gfserver_main.c
  └── readme.md
```
