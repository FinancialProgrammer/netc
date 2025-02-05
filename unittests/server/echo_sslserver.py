import socket
import ssl

# Set the server address and port
HOST = '127.0.0.1'  # Change to '0.0.0.0' to listen on all interfaces
PORT = 10000

# SSL certificate and key files
CERT_FILE = "certs/server.crt"
KEY_FILE = "certs/server.key"

def start_server():
    # Create an SSL context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=CERT_FILE, keyfile=KEY_FILE)

    # Create a socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
        sock.bind((HOST, PORT))
        sock.listen(5)
        print(f"Listening for connections on {HOST}:{PORT}...")

        # Wrap the socket with SSL
        with context.wrap_socket(sock, server_side=True) as ssock:
            while True:
                # Accept a connection
                conn, addr = ssock.accept()
                print(f"Connection from {addr}")

                # Handle the connection
                with conn:
                    while True:
                        data = conn.recv(1024)
                        if not data:
                            break
                        print(f"Received: {data.decode()}")

                        # Echo the data back to the client
                        conn.sendall(data)

if __name__ == "__main__":
    start_server()
