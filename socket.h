#pragma once

class Socket
{
public:
    Socket();
    Socket(const char *addr, const char *port);
    ~Socket();

    bool Send(unsigned char * msg, size_t lenMsg);
    bool Connect(const char *addr, const char *port);

    static bool ValidateSocket(Socket sckt);

private:

    bool GetHostAddrInfo(const char *address, const char *port);
    void DisplayConnectionInfo();
    bool ConnectToServer();
    bool login(const char *address, const char *port);
    unsigned char *GenerateHandShake(const char *address, const char *port, size_t *length);
    char *getName(size_t *length);

private:
    int socketId;
    struct addrinfo *hostAddrInfo = nullptr;
};

