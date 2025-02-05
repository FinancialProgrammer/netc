import socket

def test_raw_echo_server(host, port, message):
  try:
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the server's address
    server_address = (host, port)
    sock.connect(server_address)

    try:
      # Send data
      print(f'Sending: {message}')
      sock.sendall(message.encode())

      # Look for the response
      response = sock.recv(1024)
      print(f'Received: {response.decode()}')
    finally:
      # Close the socket
      sock.close()

  except Exception as e:
    print(f'An error occurred: {e}')

if __name__ == "__main__":
  test_raw_echo_server('localhost', 10000, 'Hello, Echo Server!')
