#include <nc/socket.hpp>

#include <stdio.h>

#define CheckError \
  if (s != nc::status::good) {  \
    printf("%s %c", nc::errString(s), '\n'); \
    return 1;  \
  }

int main() {
  nc::SocketSSL::init();
  
  nc::SocketSSL sock;
  nc::status s;

  s = sock.initIPv4(); CheckError
  s = sock.openIPv4(8443,"127.0.0.1"); CheckError // google.com (at the moment of writing update for your own test) (it is easy to get by 'ping google.com')

  s = sock.timeout(3,0); CheckError // timeout 1 second

  char sendMsg[] = "Hello, World!";
  ssize_t bsent = sock.send(sendMsg, sizeof(sendMsg));
  printf("bytes sent: %zd\n", bsent);

  char buffer[1024];
  ssize_t brecved = sock.recv(buffer,sizeof(buffer));
  printf("bytes recieved: %zd\n", brecved);

  fwrite(buffer,brecved,1,stdout);

  s = sock.close(); CheckError

  nc::SocketSSL::clean();
}