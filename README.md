# Distributed Systems Project

## Overview

This project is a practical implementation of concepts from **Distributed Systems**. Rather than having everything run inside a single program or on a single computer, the project explores how multiple independent programs can communicate and work together over a network.

The main idea is to simulate situations that occur in real-world distributed applications: users accessing files through different services, computers communicating with each other, messages being exchanged between users, and servers keeping track of which users or services are currently available.

The project is divided into two main parts, each demonstrating a different type of distributed architecture.

---

# Part 1 — Distributed File Service

The first part focuses on building a system where a user can interact with files that are managed by a remote service.

Instead of the client directly accessing the server's files, several components cooperate to process the request.

A simplified example would be:

> A user wants to download a file.  
> The client asks the system for the file, another service processes the request, and the file data is transferred back to the client.

This resembles the architecture behind many real applications where a frontend, backend services, and communication systems work together instead of one program doing everything.

## What the system demonstrates

The system separates responsibilities between different components:

- **Client** — represents the user interacting with the system.
- **Web/server component** — receives and processes requests.
- **Remote service** — allows components to communicate with each other remotely.
- **Message broker** — allows requests and responses to be exchanged asynchronously.
- **File system** — stores the actual files.

The repository contains separate client and server components and includes the RabbitMQ Java libraries used for message-based communication.

### Why use multiple components?

A distributed architecture allows different parts of an application to operate independently.

For example, if processing a file request takes some time, the client does not necessarily have to remain directly connected to the component doing the processing. A message can be placed into a queue and processed by another component.

This is similar to how large-scale applications handle background jobs, file processing, notifications, and other tasks.

---

# Part 2 — Peer-to-Peer Communication System

The second part focuses on communication between users.

The project implements a small **peer-to-peer chat system** where users can communicate with one another while a central server/broker helps them discover each other.

A simplified example is:

> Alice starts the application and connects to the broker.  
> Bob does the same.  
> Alice wants to talk to Bob, so the system finds Bob's network address.  
> Alice can then communicate directly with Bob.

This is different from a traditional system where every message would necessarily travel through one central server.

The repository contains a C implementation consisting of a peer client, server, and communication protocol.

## What the system demonstrates

The peer-to-peer system deals with several practical distributed-system problems:

- discovering other users;
- keeping track of connected peers;
- establishing communication between peers;
- sending messages over a network;
- detecting whether a peer is still active;
- handling users joining and leaving the system.

The peer client maintains a connection to the broker and can query it for another user's network endpoint.

---

# Keeping Track of Active Users

One important problem in distributed systems is knowing whether another computer or application is still available.

The project addresses this using periodic **PONG messages**.

A peer periodically informs the broker that it is still active. The broker can therefore maintain information about which peers are currently available.

In practical terms, this is similar to a person periodically saying:

> "I'm still here."

If these messages stop arriving, the system can eventually consider the peer unavailable.

The implementation includes a configurable PONG interval and sends these messages through the broker.

---

# How the Two Parts Relate to Distributed Systems

Although the two parts have different purposes, they demonstrate the same fundamental idea:

**different programs running independently need to cooperate through communication.**

This introduces challenges that do not exist in a simple standalone application.

For example:

- What happens if another computer is unavailable?
- How does one program find another program?
- How can two programs exchange information?
- How can a system know whether another component is still running?
- How can requests be handled without requiring every component to remain directly connected?
- How should different communication mechanisms be used for different purposes?

These are central questions in distributed-system design.

---

# Technical Architecture

At a higher technical level, the repository demonstrates several distributed communication technologies.

## Java Distributed File System

Part 1 uses several layers of communication:

```text
             ┌─────────────────┐
             │      Client     │
             └────────┬────────┘
                      │
                      │
                 Network/RMI
                      │
                      ▼
             ┌─────────────────┐
             │   Server/Web    │
             └────────┬────────┘
                      │
                      │ Message Queue
                      ▼
             ┌─────────────────┐
             │    RabbitMQ     │
             └────────┬────────┘
                      │
                      ▼
             ┌─────────────────┐
             │ File Service    │
             └─────────────────┘
```

The Java client contains an RMI implementation that exposes remote methods and uses a blocking queue to coordinate communication between RMI requests and the client-processing thread.

This demonstrates an important distributed-systems pattern:

**communication between components does not necessarily have to be synchronous.**

A message can be placed into a queue and processed independently.

---

# Peer-to-Peer Architecture

Part 2 follows a different architecture:

```text
                    ┌──────────────┐
                    │    Broker    │
                    │    Server    │
                    └──────┬───────┘
                           │
                 ┌─────────┴─────────┐
                 │                   │
              discovery           discovery
                 │                   │
                 ▼                   ▼
          ┌─────────────┐     ┌─────────────┐
          │    Peer A   │◄───►│    Peer B   │
          └─────────────┘     └─────────────┘
                 ▲                   ▲
                 │                   │
               PONG                PONG
                 │                   │
                 └─────────┬─────────┘
                           │
                     availability
```

The broker is used for coordination and peer discovery, while the peers can communicate directly with each other.

The C implementation uses separate protocol, server, and peer-client components.

---

# Communication Technologies

The project demonstrates multiple approaches to communication.

### Java RMI

**Remote Method Invocation (RMI)** allows one Java program to call methods that are implemented by another Java process.

Conceptually, it allows code such as:

```text
client → remoteService.someMethod()
```

even though `someMethod()` is executed in another process.

The project uses RMI interfaces and an implementation based on `UnicastRemoteObject`.

### RabbitMQ

RabbitMQ is used as a **message broker**.

Instead of components communicating directly all the time, one component can place a message into a queue and another component can process it.

This provides a form of asynchronous communication and decouples the components.

The repository includes RabbitMQ's Java client library in the Part 1 classes directory.

### TCP

TCP provides reliable communication between network applications.

It is useful when the system needs an established connection and reliable delivery of information.

The peer client uses TCP to communicate with the broker when performing operations such as joining, querying another user, and sending heartbeat information.

### UDP

UDP is used for peer messaging.

Unlike TCP, UDP does not establish a persistent connection before sending each message. It provides a lightweight mechanism for sending datagrams between peers.

The peer client receives chat messages using `recvfrom()`, demonstrating the datagram-based communication model.

---

# Main Distributed-System Concepts Demonstrated

This project can be viewed as a practical demonstration of the following concepts:

| Concept | Where it appears |
|---|---|
| Distributed components | Both parts |
| Client-server architecture | Part 1 and Part 2 |
| Peer-to-peer communication | Part 2 |
| Remote procedure calls | Java RMI |
| Message queues | RabbitMQ |
| Asynchronous communication | RabbitMQ |
| Network communication | TCP/UDP |
| Service discovery | Part 2 broker |
| Heartbeats / liveness detection | PONG messages |
| Inter-process communication | Both parts |
| Concurrency | Java blocking queues and network processes |
| Network protocols | Custom C protocol |

---

# Repository Structure

The main implementation is located under `delivery2`.

```text
delivery2/
│
├── p1/
│   │
│   ├── client/
│   │   ├── Client.java
│   │   ├── ClientImpl.java
│   │   └── ClientInterface.java
│   │
│   ├── classes/
│   │   ├── RabbitMQ libraries
│   │   └── supporting libraries
│   │
│   ├── srvwww/
│   │   ├── FileSystemActions.java
│   │   └── SrvWWW.java
│   │
│   └── supporting scripts
│
└── p2/
    │
    ├── peer-client.c
    ├── server.c
    ├── protocol.c
    ├── protocol.h
    └── Makefile
```

The repository currently contains six commits and is organized around these two delivery components.

---

# In Simple Terms

If the technical details are ignored, the project can be summarized as:

> **A collection of small distributed applications designed to demonstrate how independent computers and programs can communicate, exchange information, discover one another, and coordinate their work over a network.**

The first part demonstrates **distributed file access and asynchronous service communication**, while the second demonstrates **network-based peer-to-peer communication and user discovery**.

Together, they provide practical experience with the problems involved when software is no longer running as one isolated program, but instead consists of multiple independent components that must cooperate.

---

# Technologies Used

- **Java**
- **C**
- **Java RMI**
- **RabbitMQ**
- **TCP/IP**
- **UDP**
- **Network sockets**
- **Message queues**
- **Concurrent/blocking queues**
- **Custom network protocols**
- **Makefiles / compilation scripts**

---

# Project Purpose

The primary purpose of this project is educational: to gain practical experience designing and implementing systems where multiple independent processes communicate over a network.

Rather than simply implementing an application that works on one computer, the project explores what happens when the application is divided into multiple services and machines.

This makes it a practical example of **distributed systems, network programming, inter-process communication, service discovery, asynchronous messaging, and peer-to-peer communication**.