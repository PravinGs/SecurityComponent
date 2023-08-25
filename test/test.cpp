#include "service/commandservice.hpp"

int main()
{
    ICommand * service = new Command();
    service->readCommand("last -n 5");
    delete service;
    return 0;
}