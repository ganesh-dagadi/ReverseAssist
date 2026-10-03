#include "Logging.hpp"
#include <ServiceCore.hpp>

#include <signal.h>
#include <pthread.h>

int main() {
    Logger::getInstance().info("Main: Start");

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGUSR1);
    if (pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
        Logger::getInstance().error("Main: Failed to block control signals");
        return 1;
    }

    Logger::getInstance().info("Main: Starting Service core");
    ServiceCore service;
    service.startServiceCore();

    for (;;) {
        int receivedSignal = 0;
        const int result = sigwait(&signals, &receivedSignal);
        if (result != 0) {
            Logger::getInstance().error("Main: Failed while waiting for a control signal");
            return 1;
        }

        if (receivedSignal == SIGINT) {
            Logger::getInstance().info("Main: SIGINT received; stopping Service core");
            service.stopServiceCore();
        } else if (receivedSignal == SIGUSR1) {
            Logger::getInstance().info("Main: SIGUSR1 received; resuming Service core");
            service.resumeServiceCore();
        }
    }

    return 0;
}