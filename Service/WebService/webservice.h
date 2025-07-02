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

        // --- شروع تغییرات برای بهینه‌سازی بارگذاری پیکربندی --- //ُ]ي] 
    // اضافه کردن متغیرهای عضو برای ذخیره پیکربندی‌ها
    Configurate::FieldsStruct m_hasInputFields;
    Configurate::FieldsStruct m_hasOutputFields;
    Configurate::StoreImageConfigStruct m_StoreImageConfig;
    std::unordered_map<int, Configurate::ViolationStruct> m_ViolationMap;
    std::vector<Configurate::CameraStruct> m_Cameras;
    Configurate::InfoDatabaseStruct m_InsertDatabaseInfo;
    Configurate::InfoDatabaseStruct m_FailedDatabaseInfo;
    Configurate::ModulesStruct m_Modules;
    std::shared_ptr<MongoDB> m_InsertDatabase;
    std::shared_ptr<MongoDB> m_FailedDatabase;
    std::shared_ptr<MongoDB> m_ConfigDatabase; // برای به‌روزرسانی MinId در Meta
    Configurate::InfoDatabaseStruct m_ConfigDatabaseInfo;
    // --- پایان تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---

protected:
};

#endif //WEBSERVICE_H