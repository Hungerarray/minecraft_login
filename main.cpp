#include "pch.h"

bool validate(int argc);

int main(int argc, char **argv)
{
    CRTERR(validate(argc));

    Socket sock;
    CRTERR(!Socket::ValidateSocket(sock));
    CRTERR(!sock.Connect(argv[1], argv[2]));
    sleep(60);

    return 0;
}

bool validate(int argc)
{
    if (argc < 3)
    {
        fprintf (stderr, "usuage ./login [ip] [port] ([connections])\n\n");
        return 1;
    }
    return 0;
}


