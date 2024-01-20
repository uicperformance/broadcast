# An event-based server with epoll

## Introduction
In this assignment, we build a small socket server using epoll. 
You are given a working multi-threaded server, as well as a client that provides 
some challenging test behaviors.

## Prerequisites

Although any Linux machine will be fine for this assignment, you may use the class server {\tt nodes} to do this homework. 
It is accessed as follows:

`ssh -p 8076 YOUR\_NETID\_HERE@bits-head.cs.uic.edu` with your UIC password. Add this to your .ssh/config so that you can just type `ssh nodes` on the terminal to log in. Also, set up passwordless ssh login, so that you don't have to type in your password every time. If this sounds unnecessary, {\bf do it anyway.} Thank me later. Ask your classmates or google how if you need help with this part, but don't skip it. 

For working on nodes, I recommend that you use Visual Studio Code on your own machine, with the [Remote - SSH extension](https://code.visualstudio.com/docs/remote/ssh) to edit files on the server, rather than command line editors like vi or nano. It's really slick.

You may also want to set up your own environment, so that you still have the tools available to you when class is over. You will need the following:


- Optionally create a virtual machine with sufficient storage for Ubuntu 22.04 and some additional programs. 5 GB should be ample.
- Install Ubuntu 22.04 Desktop\footnote{Ubuntu Desktop is a great choice, with lots of good free software for it. However if you, like me, prefer to work on a Mac desktop, you can install Ubuntu 22.04 Server instead, and ssh to the VM from your mac. I can't help you with this part if you're a Windows user.}
- Install the build-essentials package {\tt apt-get install build-essentials}

## Server behavior

This GIT repository contains the template code. There is a server.c, as well as small Rust project containing a test client.

To build server.c, simply use `make server`. This is a multi-threaded server that meets
all the requirements for server behavior. Except, of course, it is multi-threaded.

Your job is to provide the same behavior, but with a single threaded server. That is,
don't use `pthread_create` or anything like that. Instead, you are to use `epoll`` to
listen to (and write to) multiple sockets at the same time. 

The server listens for incoming connections on the port specified by `-p`. 
For each connected socket (there may be a few), it listens for incoming messages.
A message is a single line of text, up to 255 bytes. 
When it has received a full message, it immediately sends that message to all the other
connected clients. 
