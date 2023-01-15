### TCP client & server (C++20 & Boost.Asio)

[![Build Status](https://github.com/StackOverflowExcept1on/tcp-daemon-for-linux/actions/workflows/ci.yml/badge.svg)](https://github.com/StackOverflowExcept1on/tcp-daemon-for-linux/actions/workflows/ci.yml)

[![asciicast](https://asciinema.org/a/zxX8zMCGW5bf8zSJtSe1Z4EI5.svg)](https://asciinema.org/a/zxX8zMCGW5bf8zSJtSe1Z4EI5)

This is one of the cool tasks from my university

### Task description

Implement a multi-threaded client-server application under Linux.

- Client is a program launched from the console
- The server is a daemon that gracefully terminates on `SIGTERM` and `SIGHUP` signals
- The client must transfer the contents of the text file via TCP
- The server must accept connection and save TCP stream to a file

### Requirements

- cmake
- conan

### Building

```bash
./build.sh
```

### Integration with CLion

By default, this may result in an error, to fix this:
1. execute `cd cmake-build-debug && conan install ..`
2. refresh CMake build

### Server

```bash
# run server at port 1234
./build/bin/server 1234

# check that daemon started
lsof -i :1234

# connect to server using netcat
echo "some data" | nc -N 0.0.0.0 1234

# read logs of the daemon
grep tcp_server_daemon /var/log/syslog

# clear syslog
sudo truncate -s 0 /var/log/syslog
```

### Client

```bash
# generate random file
head -c 1G </dev/urandom > large
sha256sum large

# send file to the server
./build/bin/client 127.0.0.1 1234 large
```
