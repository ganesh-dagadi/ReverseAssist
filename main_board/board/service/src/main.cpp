#include <iostream>
#include <ServiceCore.hpp>
#include <signal.h>
using namespace std;

ServiceCore service;
void signalHandler(int signal) {
    cout << "Signaling" << endl;
    service.stopServiceCore();
}

int main() {
    signal(SIGINT, signalHandler);
    service.startServiceCore();
    return 0;
}