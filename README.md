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

To build server.c, simply use `make server`. This is a multi-threaded server that (hopefully) meets the requirements for server behavior. Except, of course, it is multi-threaded.

Your job is to provide the same behavior, but with a single threaded server. That is,
don't use `pthread_create` or anything like that. Instead, you are to use `epoll`` to
listen to (and write to) multiple sockets at the same time. 

The server listens for incoming connections on the port specified by `-p`. 
For each connected socket (there may be a few), it listens for incoming messages.
A message is a single line of text, up to 255 bytes. 
When it has received a full message, it immediately sends that message to all the other
connected clients. 

## a client for testing

The `client` folder has a handy client for checking correct behavior under a variety of 
circumstances. To run it, use 

```cargo run -- --port 5678```

where `5687` is a number you choose, above 1024 and below 32768. Use that port for both the server and the client. If you use the same port number as someone else in class, you'll have problems, so
pick something unique.

The client takes a number of parameters. Try `cargo run -- --help` to learn more. 
You may need to [install rust](https://www.rust-lang.org/tools/install). 

## challenges

The clients are weird, in that they don't always send full messages in one go, and they can be slow
at reading. This makes life harder as an epoll server. Try running the client with `-P 0 -r 0` at first, which provides nice client behavior. 

Once they stop complaining about incorrect messages, drop the `-P 0` to start seeing a few partially transmitted messages. And eventually introduce a larger `-r` to introduce some
slow reading as well.

## expectations

You can use whatever tools or help you want to build this program. You could consider starting with
the epoll program I showed in class, but that's not at all a requirement.

Keep in mind that if you lean to heavily on others, or on AI tools, you're likely not learning much. In the midterm later, you will need the skills and knowledge you develop here.

Sometimes, the right skill is to know how to use an AI tool to get something done fast. Other times, you need to understand how things actually work, to get to the goal. So take this as
an opportunity to explore the problem space and learn different ways of getting the job done.

## some ideas to get you started

Try using `nc localhost PORTNUMBER` to listen in to what the server is sending. You can even
type in that window and the server should echo that to other nc clients, but of course, the
test client won't like what you say unless you write exactly the message it expects.

That works great when the server is sending the wrong stuff - you can see what's happening.
When the server _isn't sending anything_, listening in won't help. Here are some other things you can try:

- use `strace` to see what the server is waiting for at the moment. 
- use `gdb` to step through how the server gets into whatever state it is stuck in.
- use `tcpdump -i lo port PORTNUMBER` to see what data is being sent over sockets (a different
view than strace)

There are no performance expectations on this assignment, except that it should keep making progress, quickly forward messages, and handle a variety of client settings (including the default setting), with multiple clients (but no more than 10). 

If you have a couple of clients running, try connecting with two `nc` clients as well, and type in one to see if the other receives it. The test clients will complain, but that's ok. 

Keep in mind that using AI tools is legitimate in this class. It's _not_ always the best way to learn - they lie, they don't really know what's going on, and if you let them do your work for you, you probably won't know what's going on either in the end. That said, they're also incredibly good tools, so don't dismiss them, just try not to depend on them. 