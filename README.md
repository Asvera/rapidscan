# Simple Port Scanner

This is a basic TCP port scanner written in C++. It scans a range of ports on a specified IP address and reports which ports are open.

## Features

- Scans TCP ports in a user-defined range.
- Uses non-blocking sockets with a timeout to avoid hanging.
- Only shows open ports.
- Provides a help option.

## Requirements

- A C++ compiler (tested with g++ on Linux).
- Compatible with Linux. (Windows might requires minor adjustments.)

## Usage

Compile the program:

```bash
g++ portscanner.cpp -o portscanner
