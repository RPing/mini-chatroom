# mini-chatroom
Intro. to Network Programming 2014 Fall

A mini chatroom program using TCP socket.

### Platform
Unix-like System

### Usage
**Server**

  A server can serve multiple clients simultaneously. After a connection is built, server will send a hello message to the client, and give the client a username 'anonymous'. Clients can send different commands to server.

  First, run the program and specify the port to listen:

```
./chatroom_server <LISTEN PORT>
```

  Messages which is transmitted between clients and the server are as following:

--------------------------------------------------------------------------------
* Hello Message

When a client connects to server, the server sends hello message to the client, and broadcast this user's coming to other clients.


&nbsp;&nbsp;&nbsp;&nbsp;To the new connected client:

    [Server] Hello, anonymous! From: <Client IP>/<Client Port>

&nbsp;&nbsp;&nbsp;&nbsp;To existing client:

    [Server] Someone is coming!

--------------------------------------------------------------------------------
* Offline Message

When a client disconnect to server, the server sends offline message to all the other online clients to tell them someone has been offline.

```
[Server] <USERNAME> is offline.
```

--------------------------------------------------------------------------------
* Who Message

  A client can type command below to list all online users.

```
who
```

&nbsp;&nbsp;&nbsp;&nbsp;Server will reply to sender a list of online users and tag the sender client.
&nbsp;&nbsp;&nbsp;&nbsp;For N user, Server will send N lines.
&nbsp;&nbsp;&nbsp;&nbsp;Each of them shows details of a user.

```
[Server] <USERNAME> <CLIENT IP>/<CLIENT PORT>
[Server] <SENDER USERNAME> <CLIENT IP>/<CLIENT PORT> ->me
```

--------------------------------------------------------------------------------
* Change Username Message

  A client can type command below to change his/her username.

```
name <NEW USERNAME>
```

&nbsp;&nbsp;&nbsp;&nbsp;Server has to verify if the new name is valid which means the input name is
&nbsp;&nbsp;&nbsp;&nbsp;(1) not anonymous, (2) unique, and (3) 2~12 English letters.
&nbsp;&nbsp;&nbsp;&nbsp;It will reject user's request if this name cannot fit the rule.


&nbsp;&nbsp;&nbsp;&nbsp;If the new name is anonymous.

    [Server] ERROR: Username cannot be anonymous.

&nbsp;&nbsp;&nbsp;&nbsp;If the new name is not unique.

    [Server] ERROR: <NEW USERNAME> has been used by others.

&nbsp;&nbsp;&nbsp;&nbsp;If the new name does not consist of 2~12 English letters.

    [Server] ERROR: Username can only consists of 2~12 English letters.

&nbsp;&nbsp;&nbsp;&nbsp;Server will reply some messages to all users once a user changes his/her name.

&nbsp;&nbsp;&nbsp;&nbsp;To user which changed his/her name

    [Server] You're now known as <NEW USERNAME>.

&nbsp;&nbsp;&nbsp;&nbsp;To other users

    [Server] <OLD USERNAME> is now known as <NEW USERNAME>.

--------------------------------------------------------------------------------
* Private Message

  A client can send a private message to a specific client.

```
tell <USERNAME> <MESSAGE>
```

&nbsp;&nbsp;&nbsp;&nbsp;Server will send an error message back to the sender if either the sender's name or the
&nbsp;&nbsp;&nbsp;&nbsp;receiver's name is anonymous.

&nbsp;&nbsp;&nbsp;&nbsp;If the sender's name is anonymous

    [Server] ERROR: You are anonymous.

&nbsp;&nbsp;&nbsp;&nbsp;If the receiver's name is anonymous

    [Server] ERROR: The client to which you sent is anonymous.

&nbsp;&nbsp;&nbsp;&nbsp;If the receiver doesn't exist

    [Server] ERROR: The receiver doesn't exist.

&nbsp;&nbsp;&nbsp;&nbsp;Otherwise, the server sends the private message to the specific client and sends back a
&nbsp;&nbsp;&nbsp;&nbsp;notification to the sender.


&nbsp;&nbsp;&nbsp;&nbsp;To sender whose message is sent:

    [Server] SUCCESS: Your message has been sent.

&nbsp;&nbsp;&nbsp;&nbsp;To receiver(if both client's name are not anonymous):

    [Server] <SENDER USERNAME> tell you <MESSAGE>

--------------------------------------------------------------------------------
* Broadcast Message

  A client can send a broadcast message to all clients.

```
yell <MESSAGE>
```

&nbsp;&nbsp;&nbsp;&nbsp;While receiving the command from a user, server adds `<SENDER USERNAME>` at the head of
&nbsp;&nbsp;&nbsp;&nbsp;it and broadcasts to all users including the sender.

```
[Server] <SENDER USERNAME> yell <MESSAGE>
```

--------------------------------------------------------------------------------
* Error Command

  Commands which haven't been declared above are error commands.
  When a server receives an error command, it should send an error message back to the sending client.

```
[Server] ERROR: Error command.
```

--------------------------------------------------------------------------------

**Client**

  A client cannot connect to more than one server at the same time.
  Users should give server IP and Port as the arguments of client program.
  If the number of arguments is not two, the client program should terminate.

```
./chatroom_client <SERVER IP> <SERVER PORT>
```

--------------------------------------------------------------------------------
* Exit Command

  User can type command below to terminate the process at any time.

```
exit
```

### TODO
GUI
