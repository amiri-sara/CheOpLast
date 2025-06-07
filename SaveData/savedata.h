#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Time/timetools.h"

//#include "../Database/MongoDB.h" // اضافه کردن برای استفاده از MongoDB::Field و MongoDB

#include <vector>
#include <mutex> // برای محافظت از بافر
#include <atomic> // برای پرچم‌های اتمیک
#include <chrono> // برای تایمر
#include <boost/asio.hpp> // برای async_wait و io_service
#include <boost/thread.hpp> // برای boost::thread
#ifdef KAFKAOUTPUT
    #include "../Kafka/KafkaProsumer.h"
    #include "../Kafka/Utility.h"
#endif // KAFKAOUTPUT

class savedata
{
public:
    savedata(); // اضافه کردن سازنده پیش‌فرض
    ~savedata(); // اضافه کردن تخریب‌کننده برای پاکسازی منابع
    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

#if defined KAFKAOUTPUT
    void setOutputKafkaConnection(std::shared_ptr<KafkaProsumer> OKC){this->OutputKafkaConnection = OKC;}
#endif // KAFKAOUTPUT
private:  
    bool InsertDatabase(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

#ifdef KAFKAOUTPUT
    bool InsertKafka(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    std::shared_ptr<KafkaProsumer> OutputKafkaConnection;
#endif // KAFKAOUTPUT

    // --- اعضای جدید برای مدیریت بافر و فلش دسته‌ای ---
    std::vector<std::vector<MongoDB::Field>> m_documents_buffer; // بافر برای نگهداری اسناد
    std::mutex m_buffer_mutex; // mutex برای محافظت از بافر در هنگام دسترسی‌های همزمان
    std::atomic<bool> m_running_flush_thread; // پرچم برای کنترل چرخه حیات رشته تایمر (وقتی false شود، رشته متوقف می‌شود)
    std::atomic<bool> m_is_flushing; // پرچم برای جلوگیری از اجرای همزمان دو عملیات فلش
    
    const size_t BULK_INSERT_THRESHOLD = 100; // آستانه (تعداد اسناد) برای شروع درج دسته‌ای
    const int FLUSH_INTERVAL_SECONDS = 1; // فاصله زمانی (بر حسب ثانیه) برای فلش دوره‌ای بافر

    boost::asio::io_service m_io_service; // سرویس I/O برای مدیریت عملیات ناهمگام (مانند تایمر)
    std::unique_ptr<boost::asio::deadline_timer> m_flush_timer; // شیء تایمر برای زمان‌بندی فلش‌های دوره‌ای
    boost::thread m_io_thread; // رشته‌ای که io_service را اجرا می‌کند

    // اعضای برای نگهداری اینستنس MongoDB و اطلاعات آن (برای استفاده در flushBuffer)
    std::shared_ptr<MongoDB> m_insert_database_client; 
    Configurate::InfoDatabaseStruct m_insert_database_info;

        // --- اضافه کردن work_guard ---
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work_guard;
    // --- پایان اضافه کردن work_guard ---
    // --- پایان اعضای جدید ---

    // متد خصوصی برای فلش کردن بافر
    void flushBuffer(bool force_flush = false);
    // متد خصوصی برای شروع تایمر فلش
    void startFlushTimer();
};



#endif //SAVEDATA_H