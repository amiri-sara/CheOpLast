#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include "../Service.h"

class WebService : public Service
{
public:
    WebService(Configurate::WebServiceConfigStruct ServiceConfig);
    void run() override;

private:    
    std::shared_ptr<crow::SimpleApp> app;
    Configurate::WebServiceConfigStruct WebServiceConfig;

    void InsertRoute();
    void TokenRoute();

        // Thread pool components
    // class ThreadPool {
    // public:
    //     ThreadPool(size_t numThreads);
    //     ~ThreadPool();
    //     void enqueue(std::function<void()> task);
    //     void stop();

    // private:
    //     std::vector<std::thread> workers;
    //     std::queue<std::function<void()>> tasks;
    //     std::mutex queueMutex;
    //     std::condition_variable condition;
    //     bool stopFlag = false;
    // };

    // std::unique_ptr<ThreadPool> threadPool;

protected:
};

#endif //WEBSERVICE_H