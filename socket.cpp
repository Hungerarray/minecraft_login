#include "pch.h"

Socket::Socket() = default;

Socket::Socket(const char *address, const char *port)
{
    Connect(address, port);
}

bool Socket::Connect(const char *address, const char *port)
{
    //get host address info
    if (!GetHostAddrInfo(address, port))
        return false;

    // display info
    DisplayConnectionInfo();

    if (!ConnectToServer())
        return false;

    if (!login(address, port))
        return false;

    return true;
}

bool Socket::ConnectToServer()
{
    socketId = socket(hostAddrInfo->ai_family, hostAddrInfo->ai_socktype, hostAddrInfo->ai_protocol);

    if (connect(socketId, hostAddrInfo->ai_addr, hostAddrInfo->ai_addrlen))
    {
        fprintf(stderr, "inital connection to server failed (%d)\n\n", errno);
        return false;
    }

    return true;
}

bool Socket::ValidateSocket(Socket sckt)
{
    if (sckt.socketId < 0)
    {
        fprintf(stderr, "Failed to create a valid socket.\n\n");
        return false;
    }
    else
        return true;
}

Socket::~Socket()
{
    close(socketId);
    freeaddrinfo(hostAddrInfo);
}

// returns false if error
bool Socket::GetHostAddrInfo(const char *address, const char *port)
{
    // setting up data for connection
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(address, port, &hints, &hostAddrInfo))
    {
        fprintf(stderr, "Socket Connect : failed to get server address.\n\n");
        socketId = -1;
        hostAddrInfo = nullptr;
        return false;
    }
    return true;
}

void Socket::DisplayConnectionInfo()
{
    // displaying the connection
    printf("Connecting to: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(hostAddrInfo->ai_addr, hostAddrInfo->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("%s | on port: %s\n\n", address_buffer, service_buffer);
}

// used to convert from integer to minecraft varInt type
// ! doesn't work for negative values
unsigned char *cnvrtValInt(int32_t value)
{
    unsigned char *ValInt = (unsigned char *)malloc(5);
    int i = 0;
    do
    {
        unsigned char temp = value & 0x7f;
        value = value >> 7;
        if (value)
            temp |= 0x80;
        ValInt[i++] = temp;
    } while (i < 5 && value);

    return ValInt;
}

unsigned char *Socket::GenerateHandShake(const char *address, const char *port, size_t *length)
{

    int len = strlen(address), pos = 0;
    unsigned char *data = (unsigned char *)malloc(13 + len + 3), *temp = cnvrtValInt(PROTOCOL);

    data[pos++] = 0x00; // * packet ID

    memcpy(data + pos, temp, 5); // * protocol version
    free(temp);
    pos += 5;

    temp = cnvrtValInt(len); // * server address
    memcpy(data + pos, temp, 3);
    pos += 3;
    memcpy(data + pos, address, len);
    free(temp);
    pos += len;

    len = strlen(port);
    memcpy(data + len, port, len); // * port
    pos += len;

    data[pos++] = 0x02;
    *length = pos;
    return data;
}

bool Socket::Send(unsigned char *msg, size_t lenMsg)
{
    while (true)
    {
        fd_set reads;
        fd_set writes;
        FD_ZERO(&reads);
        FD_ZERO(&writes);
        FD_SET(socketId, &writes);
        FD_SET(socketId, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socketId + 1, &reads, &writes, 0, &timeout) < 0)
        {
            fprintf(stderr, "socket: Connect: send : select() failed");
            return false;
        }

        if (FD_ISSET(socketId, &reads))
        {
            char read[4096];
            int bytes_recieved = recv(socketId, read, 4096, 0);
            if (bytes_recieved < 1)
            {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s", bytes_recieved, bytes_recieved, read);
        }

        if (FD_ISSET(socketId, &writes))
        {
            static size_t bytes_sent = 0;
            bytes_sent += send(socketId, msg, lenMsg, 0);
            if (bytes_sent >= lenMsg)
                break;
        }
    }

    return true;
}

bool Socket::login(const char *address, const char *port)
{
    size_t len;
    unsigned char *msg = GenerateHandShake(address, port, &len);

    Send(msg, len);
    free(msg);

    msg = (unsigned char *)malloc(5);
    msg[0] = 0x00;
    char *name = getName(&len);
    memcpy(msg + 1, name, len);
    free(name);

    Send(msg, len + 1);

    return true;
}

char *Socket::getName(size_t *length)
{
    static int id = 0;
    char *dat = (char *)malloc(15);
    char *temp = "Nishan_chor";
    int len = strlen(temp);
    memcpy(dat, temp, len);
    char val = '0' + id++;
    memcpy(dat + len, &val, 1);
    dat[len + 2] = '\0';
    *length = len + 3;

    return (char *)realloc(dat, *length);
}