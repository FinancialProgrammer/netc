#!/usr/bin/python3
# Usage: python3 echoTcpServer.py [bind IP] [bind PORT]

import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the provided IP and PORT
server_address = (sys.argv[1], int(sys.argv[2]))
sock.bind(server_address)

# Listen for incoming connections (1 connection at a time)
sock.listen(1)
print(f"Server is listening on {server_address}")

while True:
  # Wait for a connection
  print("Waiting for a connection...")
  connection, client_address = sock.accept()

  try:
    print("Connection established with", client_address)
    
    # Loop to continuously receive and echo messages
    while True:
      data = connection.recv(1024)  # Buffer size is 1024 bytes
      if data:
        print(f"Received: {data.decode('utf-8')}")  # Decode and print received message
        connection.sendall(data)  # Echo the message back to the client
        print(f"Sent: {data.decode('utf-8')}")
      else:
        # No more data, connection closed by client
        print("Client disconnected")
        break

  except Exception as e:
    print(f"An error occurred: {e}")
  
  finally:
    # Clean up the connection
    connection.close()
    print("Connection closed")
