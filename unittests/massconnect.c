#include <netc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For sleep function

#define NUM_CLIENTS 16

struct nc_functions G_sockfuncs;

void* client_thread(void* arg) {
    nc_error_t err;
    NCSOCKET sock;

    err = G_sockfuncs.socket(&sock, NC_OPT_IPV4, NC_OPT_SOCK_STREAM, NC_OPT_TCP);
    if (err != NC_ERR_GOOD) {
        printf("Error creating socket: %s\n", nstrerr(err));
        return NULL;
    }

    nc_socketaddr_t sockaddr;
    err = netc_resolve_addrV4(&sockaddr, NC_OPT_NULL, "127.0.0.1", 10000);
    if (err != NC_ERR_GOOD) {
        printf("Error resolving address: %s\n", nstrerr(err));
        return NULL;
    }

    err = G_sockfuncs.open(&sock, &sockaddr);
    if (err != NC_ERR_GOOD) {
        fprintf(stderr, "Error opening socket: %s\n", nstrerr(err));
        return NULL;
    }

    char hello[] = "Hello Server, I am a responsible client!";
    size_t bytes_written;
    err = G_sockfuncs.write(&sock, hello, sizeof(hello), &bytes_written, NC_OPT_DO_ALL);
    if (err != NC_ERR_GOOD) {
        printf("Error writing to socket: %s\n", nstrerr(err));
        return NULL;
    }

    char srvmsg[1024];
    size_t bytes_read;
    err = G_sockfuncs.read(&sock, srvmsg, sizeof(srvmsg) - 1, &bytes_read, NC_OPT_NULL);
    if (err != NC_ERR_GOOD) {
        printf("Error reading from socket: %s\n", nstrerr(err));
        return NULL;
    }

    srvmsg[bytes_read] = '\0';
    printf("Server sent: %s\n", srvmsg);

    G_sockfuncs.close(&sock);

    return NULL;
}

int main() {
#ifdef _USEOPENSSL
    G_sockfuncs = nc_functions_openssl(); 
#else 
    G_sockfuncs = nc_functions_raw();
#endif

    pthread_t threads[NUM_CLIENTS];
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (pthread_create(&threads[i], NULL, client_thread, NULL)) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        // sleep(1); // Add a delay between client connections
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
