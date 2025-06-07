#include "savedata.h"

// سازنده: منابع جدید را مقداردهی اولیه کرده و رشته تایمر را آغاز می‌کند.
savedata::savedata() : m_running_flush_thread(true), m_is_flushing(false), m_io_thread(boost::bind(&boost::asio::io_service::run, &m_io_service))
{
    // --- اضافه کردن work_guard ---
    // این خط io_service را فعال نگه می‌دارد تا حتی بدون کارهای pending هم run() خارج نشود.
    m_work_guard = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(m_io_service.get_executor());
    // --- پایان اضافه ---

    m_flush_timer = std::make_unique<boost::asio::deadline_timer>(m_io_service);
    startFlushTimer(); // شروع تایمر برای فلش دوره‌ای
}


// تخریب‌کننده: تضمین می‌کند که رشته تایمر متوقف شده و منابع آزاد شوند.
savedata::~savedata()
{
    m_running_flush_thread.store(false); // به رشته تایمر اطلاع می‌دهد که متوقف شود.
    
    // --- قبل از stop کردن io_service، work_guard را null می‌کنیم ---
    m_work_guard.reset(); // آزادسازی work_guard تا io_service بتواند خارج شود.
    // --- پایان تغییر ---

    m_io_service.stop(); // io_service را متوقف می‌کند تا رشته آن پایان یابد.
    if (m_io_thread.joinable()) {
        m_io_thread.join(); // منتظر می‌ماند تا رشته تایمر کار خود را تمام کند.
    }
}

bool savedata::run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{

    // --- در اولین فراخوانی `run`، client مربوط به MongoDB را ذخیره می‌کنیم ---
    // این کار تضمین می‌کند که m_insert_database_client و m_insert_database_info فقط یک بار تنظیم شوند.
    // همچنین، در اینجا چک می‌کنیم که DH->InsertDatabase معتبر باشد.
    static std::once_flag init_flag;
    std::call_once(init_flag, [&]() {
        if (DH->InsertDatabase) { // بررسی معتبر بودن shared_ptr
            m_insert_database_client = DH->InsertDatabase;
            m_insert_database_info = DH->InsertDatabaseInfo;
        } else {
            Logger::getInstance().logError("DH->InsertDatabase is null. Bulk insert will not work.");
        }
    });

#ifdef INSERTDATABASE
    // Insert Database
    if(!(this->InsertDatabase(DH)))
        return false;
#endif // INSERTDATABASE

#ifdef KAFKAOUTPUT
    // Insert Kafka
    if(!(this->InsertKafka(DH)))
        return false;
#endif // KAFKAOUTPUT
    
    return true;
}

// --- پیاده‌سازی متد flushBuffer ---
void savedata::flushBuffer(bool force_flush)
{
    Logger::getInstance().logInfo("FlushBuffer called. force_flush: " + std::to_string(force_flush));
    if (m_is_flushing.exchange(true)) { 
        Logger::getInstance().logWarning("Flush already in progress. Skipping current flush request.");
        return; 
    }

    std::vector<std::vector<MongoDB::Field>> documents_to_insert;
    {
        std::lock_guard<std::mutex> lock(m_buffer_mutex); 
        Logger::getInstance().logInfo("Buffer locked. Current size: " + std::to_string(m_documents_buffer.size()));

        if (!force_flush && m_documents_buffer.size() < BULK_INSERT_THRESHOLD && m_running_flush_thread.load()) {
            Logger::getInstance().logInfo("Buffer size below threshold. Not flushing yet.");
            m_is_flushing.store(false); 
            return; 
        }
        if (m_documents_buffer.empty()) {
            Logger::getInstance().logInfo("Buffer is empty. Nothing to flush.");
            m_is_flushing.store(false); 
            return; 
        }
        documents_to_insert.swap(m_documents_buffer); 
        Logger::getInstance().logInfo("Swapped " + std::to_string(documents_to_insert.size()) + " documents for flushing.");
    }

    if (m_insert_database_client) {
        MongoDB::ResponseStruct InsertReturn = m_insert_database_client->InsertMany(
            m_insert_database_info.DatabaseName, 
            m_insert_database_info.CollectionName, 
            documents_to_insert
        );

        if(InsertReturn.Code != MongoDB::MongoStatus::InsertSuccessful) {
            Logger::getInstance().logError("Failed bulk insert: " + InsertReturn.Description);
            // می‌توانید در اینجا برای اسنادی که درج نشدند، منطق بازیابی اضافه کنید (مثلاً به FailedDatabase)
        } else {
            Logger::getInstance().logInfo("Successfully bulk inserted " + std::to_string(documents_to_insert.size()) + " documents.");
        }
    } else {
        Logger::getInstance().logError("MongoDB client (m_insert_database_client) is null. Cannot perform bulk insert.");
    }
    m_is_flushing.store(false); 
    Logger::getInstance().logInfo("Flush operation finished. m_is_flushing set to false.");
}

// پیاده‌سازی متد startFlushTimer
void savedata::startFlushTimer()
{
    m_flush_timer->expires_from_now(boost::posix_time::seconds(FLUSH_INTERVAL_SECONDS));
    m_flush_timer->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && m_running_flush_thread.load()) {
            flushBuffer(true); // فلش اجباری بافر
            startFlushTimer(); // دوباره تایمر را برای دوره بعدی شروع کنید
        } else if (ec != boost::asio::error::operation_aborted) {
            Logger::getInstance().logError("Flush timer error: " + ec.message());
        }
    });
}
// --- پایان پیاده‌سازی متد flushBuffer ---

bool savedata::InsertDatabase(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    std::vector<MongoDB::Field> fields;

    // DeviceID
    if(DH->hasOutputFields.DeviceID)
    {
        if(DH->hasInputFields.DeviceID)
        {  
            MongoDB::Field DeviceIDField = {"DeviceID", std::to_string(DH->Input.DeviceID), MongoDB::FieldType::Integer};
            fields.push_back(DeviceIDField);
        }
    }

    // UserID
    if(DH->hasOutputFields.UserID)
    {
        if(DH->hasInputFields.UserID)
        {  
            MongoDB::Field UserIDField = {"UserID", std::to_string(DH->Input.UserID), MongoDB::FieldType::Integer};
            fields.push_back(UserIDField);
        }
    }

    // StreetID
    if(DH->hasOutputFields.StreetID)
    {
        if(DH->hasInputFields.StreetID)
        {  
            MongoDB::Field StreetIDField = {"StreetID", std::to_string(DH->Input.StreetID), MongoDB::FieldType::Integer};
            fields.push_back(StreetIDField);
        }
    }

    // ViolationID
    if(DH->hasOutputFields.ViolationID)
    {
        if(DH->hasInputFields.ViolationID)
        {  
            MongoDB::Field ViolationIDField = {"ViolationID", std::to_string(DH->Input.ViolationID), MongoDB::FieldType::Integer};
            fields.push_back(ViolationIDField);
        }
    }

    // Direction
    if(DH->hasOutputFields.Direction)
    {
        if(DH->hasInputFields.Direction)
        {  
            MongoDB::Field DirectionField = {"Direction", std::to_string(DH->Input.Direction), MongoDB::FieldType::Integer};
            fields.push_back(DirectionField);
        }
    }

    // PlateType
    if(DH->hasOutputFields.PlateType)
    {
        if(DH->hasInputFields.PlateType)
        {  
            MongoDB::Field PlateTypeField = {"PlateType", std::to_string(DH->Input.PlateType), MongoDB::FieldType::Integer};
            fields.push_back(PlateTypeField);
        }
    }

    // PlateValue
    if(DH->hasOutputFields.PlateValue)
    {
        if(DH->hasInputFields.PlateValue)
        {  
            MongoDB::Field PlateValueField = {"PlateValue", DH->Input.PlateValue, MongoDB::FieldType::String};
            fields.push_back(PlateValueField);
        }
    }

    // Suspicious
    if(DH->hasOutputFields.Suspicious)
    {
        if(DH->hasInputFields.Suspicious)
        {  
            MongoDB::Field SuspiciousField = {"Suspicious", DH->Input.Suspicious, MongoDB::FieldType::String};
            fields.push_back(SuspiciousField);
        }
    }
    
    // Speed
    if(DH->hasOutputFields.Speed)
    {
        if(DH->hasInputFields.Speed)
        {  
            MongoDB::Field SpeedField = {"Speed", std::to_string(DH->Input.Speed), MongoDB::FieldType::Integer};
            fields.push_back(SpeedField);
        }
    }

    // VehicleType
    if(DH->hasOutputFields.VehicleType)
    {
        if(DH->hasInputFields.VehicleType)
        {  
            MongoDB::Field VehicleTypeField = {"VehicleType", std::to_string(DH->Input.VehicleType), MongoDB::FieldType::Integer};
            fields.push_back(VehicleTypeField);
        }
    }

    // VehicleColor
    if(DH->hasOutputFields.VehicleColor)
    {
        if(DH->hasInputFields.VehicleColor)
        {  
            MongoDB::Field VehicleColorField = {"VehicleColor", std::to_string(DH->Input.VehicleColor), MongoDB::FieldType::Integer};
            fields.push_back(VehicleColorField);
        }
    }

    // VehicleModel
    if(DH->hasOutputFields.VehicleModel)
    {
        if(DH->hasInputFields.VehicleModel)
        {  
            MongoDB::Field VehicleModelField = {"VehicleModel", std::to_string(DH->Input.VehicleModel), MongoDB::FieldType::Integer};
            fields.push_back(VehicleModelField);
        }else{
            MongoDB::Field VehicleModelField = {"VehicleModel", std::to_string(DH->ProcessedInputData.VehicleModel), MongoDB::FieldType::Integer};
            fields.push_back(VehicleModelField);
        }
    }

    // Lane
    if(DH->hasOutputFields.Lane)
    {
        if(DH->hasInputFields.Lane)
        {  
            MongoDB::Field LaneField = {"Lane", std::to_string(DH->Input.Lane), MongoDB::FieldType::Integer};
            fields.push_back(LaneField);
        }
    }

    // PassedTime
    if(DH->hasOutputFields.PassedTime)
    {
        if(DH->hasInputFields.PassedTime)
        {
            std::ostringstream oss;
            oss << std::put_time(&(DH->ProcessedInputData.PassedTimeLocal), "%Y-%m-%d %H:%M:%S");
            std::string PT = oss.str();
            
            MongoDB::Field PassedTimeField = {"PassedTime", PT, MongoDB::FieldType::Date};
            fields.push_back(PassedTimeField);
        }
    }

    // ColorImage
    if(DH->hasOutputFields.ColorImage)
    {
        if(DH->hasInputFields.ColorImage)
        {            
            MongoDB::Field ColorImageField = {"ColorImage", DH->Input.ColorImage, MongoDB::FieldType::String};
            fields.push_back(ColorImageField);
        }
    }

    // ImageAddress
    if(DH->hasOutputFields.ImageAddress)
    {
        if(DH->hasInputFields.ImageAddress)
        {            
            MongoDB::Field ImageAddressField = {"ImageAddress", DH->Input.ImageAddress, MongoDB::FieldType::String};
            fields.push_back(ImageAddressField);
        }else
        {
            MongoDB::Field ImageAddressField = {"ImageAddress", DH->StoreImageAddress.ImageAddress, MongoDB::FieldType::String};
            fields.push_back(ImageAddressField);
        }
    }

    // GrayScaleImage
    if(DH->hasOutputFields.GrayScaleImage)
    {
        if(DH->hasInputFields.GrayScaleImage)
        {            
            MongoDB::Field GrayScaleImageField = {"GrayScaleImage", DH->Input.GrayScaleImage, MongoDB::FieldType::String};
            fields.push_back(GrayScaleImageField);
        }
    }

    // PlateImage
    if(DH->hasOutputFields.PlateImage)
    {
        if(DH->hasInputFields.PlateImage)
        {            
            MongoDB::Field PlateImageField = {"PlateImage", DH->Input.PlateImageBase64, MongoDB::FieldType::String};
            fields.push_back(PlateImageField);
        }
    }

    // Latitude
    if(DH->hasOutputFields.Latitude)
    {
        if(DH->hasInputFields.Latitude)
        {            
            MongoDB::Field LatitudeField = {"Latitude", std::to_string(DH->Input.Latitude), MongoDB::FieldType::Double};
            fields.push_back(LatitudeField);
        }
    }

    // Longitude
    if(DH->hasOutputFields.Longitude)
    {
        if(DH->hasInputFields.Longitude)
        {            
            MongoDB::Field LongitudeField = {"Longitude", std::to_string(DH->Input.Longitude), MongoDB::FieldType::Double};
            fields.push_back(LongitudeField);
        }
    }
    
    // Accuracy
    if(DH->hasOutputFields.Accuracy)
    {
        if(DH->hasInputFields.Accuracy)
        {  
            MongoDB::Field AccuracyField = {"Accuracy", std::to_string(DH->Input.Accuracy), MongoDB::FieldType::Integer};
            fields.push_back(AccuracyField);
        }
    }

    // PlateRect
    if(DH->hasOutputFields.PlateRect)
    {
        if(DH->hasInputFields.PlateRect)
        {  
            MongoDB::Field PlateRectField = {"PlateRect", DH->Input.PlateRect, MongoDB::FieldType::String};
            fields.push_back(PlateRectField);
        }
    }

    // CarRect
    if(DH->hasOutputFields.CarRect)
    {
        if(DH->hasInputFields.CarRect)
        {  
            MongoDB::Field CarRectField = {"CarRect", DH->Input.CarRect, MongoDB::FieldType::String};
            fields.push_back(CarRectField);
        }
        else{
            MongoDB::Field CarRectField = {"CarRect", DH->ProcessedInputData.CarRect, MongoDB::FieldType::String};
            fields.push_back(CarRectField);
        }
    }

    // CodeType
    if(DH->hasOutputFields.CodeType)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.CodeType)
        {  
            MongoDB::Field CodeTypeField = {"CodeType", std::to_string(DH->Input.CodeType), MongoDB::FieldType::Integer};
            fields.push_back(CodeTypeField);
            
        }
    }

    // Probability
    if(DH->hasOutputFields.Probability)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.Probability)
        {  
            MongoDB::Field ProbabilityField = {"Probability", std::to_string(DH->Input.Probability), MongoDB::FieldType::Integer};
            fields.push_back(ProbabilityField);
        }
    }

    // MasterPlate
    if(DH->hasOutputFields.MasterPlate)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.MasterPlate)
        {  
            MongoDB::Field MasterPlateField = {"MasterPlateValue", DH->Input.MasterPlate, MongoDB::FieldType::String};
            fields.push_back(MasterPlateField);
        }
    }
    //PassedVehicleRecordsId
    if(DH->hasOutputFields.PassedVehicleRecordsId)
    {
        MongoDB::Field PassedVehicleRecordsId = {"PassedVehicleRecordsId", std::to_string(DH->Input.PassedVehicleRecordsId), MongoDB::FieldType::Int64};
        fields.push_back(PassedVehicleRecordsId);
    }

    // RecordID
    if(DH->hasOutputFields.RecordID)
    {  
        if(DH->hasInputFields.DeviceID )//&& DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
        {
            MongoDB::Field RecordIDField = {"RecordID", std::to_string(DH->Input.RecordID), MongoDB::FieldType::Int64};
            fields.push_back(RecordIDField);
        }

    }

    // SystemCode
    if(DH->hasOutputFields.SystemCode)
    {
        if(DH->hasInputFields.SystemCode)
        {  
            MongoDB::Field SystemCode = {"SystemCode", std::to_string(DH->Input.SystemCode), MongoDB::FieldType::Integer};
            fields.push_back(SystemCode);
        }

    }

    // CompanyCode
    if(DH->hasOutputFields.CompanyCode)
    {
        if(DH->hasInputFields.CompanyCode)
        {  
            MongoDB::Field CompanyCode = {"CompanyCode", std::to_string(DH->Input.CompanyCode), MongoDB::FieldType::Integer};
            fields.push_back(CompanyCode);
        }

    }

    // ReceivedTime
    if(DH->hasOutputFields.ReceivedTime)
    {
        if(DH->hasInputFields.ReceivedTime)
        {
            std::ostringstream oss;
            oss << std::put_time(&(DH->ProcessedInputData.ReceivedTimeLocal), "%Y-%m-%d %H:%M:%S");
            std::string RT = oss.str();
            
            MongoDB::Field ReceivedTimeField = {"ReceivedTime", RT, MongoDB::FieldType::Date};
            fields.push_back(ReceivedTimeField);
        }else
        {
            std::ostringstream oss;
            std::time_t currentTime = std::time(nullptr);
            std::tm* CurrenttimeInfo = std::localtime(&currentTime);
            oss << std::put_time(CurrenttimeInfo, "%Y-%m-%d %H:%M:%S");
            std::string RT = oss.str();

            MongoDB::Field ReceivedTimeField = {"ReceivedTime", RT, MongoDB::FieldType::Date};
            fields.push_back(ReceivedTimeField);
        }
    }

    if(DH->Modules.Classifier.active)
    {
        for(const auto& keyLabel : DH->ProcessedInputData.ClassifierModuleOutput)
        {
            MongoDB::Field ClassifierField = {keyLabel.first, std::to_string(keyLabel.second), MongoDB::FieldType::Integer};
            fields.push_back(ClassifierField);
        }
    }


        // auto InsertReturn = DH->InsertDatabase->Insert(DH->InsertDatabaseInfo.DatabaseName, DH->InsertDatabaseInfo.CollectionName, fields);
        // if(InsertReturn.Code != MongoDB::MongoStatus::InsertSuccessful) //TODO
        // {
        //     DH->Response.HTTPCode = 500;
        //     DH->Response.errorCode = DATABASEERROR;
        //     DH->Response.Description = InsertReturn.Description;
        //     return false;
        // }

        // --- اضافه کردن سند به بافر ---
    {
        std::lock_guard<std::mutex> lock(m_buffer_mutex); // محافظت از بافر در هنگام اضافه کردن سند
        m_documents_buffer.push_back(fields); // اضافه کردن سند به بافر
    }

    // اگر تعداد اسناد در بافر به حد نصاب رسید، اقدام به فلش می‌کنیم
    if (m_documents_buffer.size() >= BULK_INSERT_THRESHOLD) {
        // این فراخوانی غیرمسدودکننده است و فلش واقعی در رشته تایمر اجرا می‌شود،
        // اما برای اطمینان از واکنش سریع به پر شدن بافر، آن را به این شکل فراخوانی می‌کنیم.
        flushBuffer(true); // فلش اجباری بافر
    }
    // --- پایان اضافه ---
    
    
      


    return true;
}

#ifdef KAFKAOUTPUT
bool savedata::InsertKafka(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    crow::json::wvalue Response;

    // DeviceID
    if(DH->hasOutputFields.DeviceID)
    {
        if(DH->hasInputFields.DeviceID)
            Response["DeviceID"] = DH->Input.DeviceID;
    }

    // UserID
    if(DH->hasOutputFields.UserID)
    {
        if(DH->hasInputFields.UserID)
            Response["UserID"] = DH->Input.UserID;
    }

    // StreetID
    if(DH->hasOutputFields.StreetID)
    {
        if(DH->hasInputFields.StreetID)
            Response["StreetID"] = DH->Input.StreetID;
    }

    // ViolationID
    if(DH->hasOutputFields.ViolationID)
    {
        if(DH->hasInputFields.ViolationID)
            Response["ViolationID"] = DH->Input.ViolationID;
    }

    // Direction
    if(DH->hasOutputFields.Direction)
    {
        if(DH->hasInputFields.Direction)
            Response["Direction"] = DH->Input.Direction;
    }

    // PlateType
    if(DH->hasOutputFields.PlateType)
    {
        if(DH->hasInputFields.PlateType)
            Response["PlateType"] = DH->Input.PlateType;
    }

    // PlateValue
    if(DH->hasOutputFields.PlateValue)
    {
        if(DH->hasInputFields.PlateValue)
            Response["PlateValue"] = DH->Input.PlateValue;
    }

    // Suspicious
    if(DH->hasOutputFields.Suspicious)
    {
        if(DH->hasInputFields.Suspicious)
            Response["Suspicious"] = DH->Input.Suspicious;
    }
    
    // Speed
    if(DH->hasOutputFields.Speed)
    {
        if(DH->hasInputFields.Speed)
            Response["Speed"] = DH->Input.Speed;
    }

    // VehicleType
    if(DH->hasOutputFields.VehicleType)
    {
        if(DH->hasInputFields.VehicleType)
            Response["VehicleType"] = DH->Input.VehicleType;
    }

    // VehicleColor
    if(DH->hasOutputFields.VehicleColor)
    {
        if(DH->hasInputFields.VehicleColor)
            Response["VehicleColor"] = DH->Input.VehicleColor;
    }

    // VehicleModel
    if(DH->hasOutputFields.VehicleModel)
    {
        if(DH->hasInputFields.VehicleModel)
            Response["VehicleModel"] = DH->Input.VehicleModel;
    }

    // Lane
    if(DH->hasOutputFields.Lane)
    {
        if(DH->hasInputFields.Lane)
            Response["Lane"] = DH->Input.Lane;
    }

    // PassedTime
    if(DH->hasOutputFields.PassedTime)
    {
        if(DH->hasInputFields.PassedTime)
            Response["PassedTime"] = DH->Input.PassedTime;
    }

    // ColorImage
    if(DH->hasOutputFields.ColorImage)
    {
        if(DH->hasInputFields.ColorImage)
            Response["ColorImage"] = DH->Input.ColorImage;
    }

    // ImageAddress
    if(DH->hasOutputFields.ImageAddress)
    {
        if(DH->hasInputFields.ImageAddress)
        {            
            Response["ImageAddress"] = DH->Input.ImageAddress;
        }else
        {
            Response["ImageAddress"] = DH->StoreImageAddress.ImageAddress;
        }
    }

    // GrayScaleImage
    if(DH->hasOutputFields.GrayScaleImage)
    {
        if(DH->hasInputFields.GrayScaleImage)
            Response["GrayScaleImage"] = DH->Input.GrayScaleImage;
    }

    // PlateImage
    if(DH->hasOutputFields.PlateImage)
    {
        if(DH->hasInputFields.PlateImage)
            Response["PlateImage"] = DH->Input.PlateImage;
    }

    // Latitude
    if(DH->hasOutputFields.Latitude)
    {
        if(DH->hasInputFields.Latitude)
            Response["Latitude"] = DH->Input.Latitude;
    }

    // Longitude
    if(DH->hasOutputFields.Longitude)
    {
        if(DH->hasInputFields.Longitude)
            Response["Longitude"] = DH->Input.Longitude;
    }
    
    // Accuracy
    if(DH->hasOutputFields.Accuracy)
    {
        if(DH->hasInputFields.Accuracy)
            Response["Accuracy"] = DH->Input.Accuracy;
    }

    // PlateRect
    if(DH->hasOutputFields.PlateRect)
    {
        if(DH->hasInputFields.PlateRect)
            Response["PlateRect"] = DH->Input.PlateRect;
    }

    // CarRect
    if(DH->hasOutputFields.CarRect)
    {
        if(DH->hasInputFields.CarRect)
            Response["CarRect"] = DH->Input.CarRect;
    }

    // CodeType
    if(DH->hasOutputFields.CodeType)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.CodeType)
            Response["CodeType"] = DH->Input.CodeType;
    }

    // Probability
    if(DH->hasOutputFields.Probability)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.Probability)
            Response["Probability"] = DH->Input.Probability;
    }

    // MasterPlate
    if(DH->hasOutputFields.MasterPlate)
    {
        if(DH->Modules.CheckOperator.active || DH->hasInputFields.MasterPlate)
            Response["MasterPlate"] = DH->Input.MasterPlate;
    }

    // RecordID
    if(DH->hasOutputFields.RecordID)
    {
        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
            Response["RecordID"] = DH->ProcessedInputData.MongoID;
    }

    // ReceivedTime
    if(DH->hasOutputFields.ReceivedTime)
    {
        if(DH->hasInputFields.ReceivedTime)
        {
            Response["ReceivedTime"] = DH->Input.ReceivedTime;
        }else
        {
            std::ostringstream oss;
            std::time_t currentTime = std::time(nullptr);
            std::tm* CurrenttimeInfo = std::gmtime(&currentTime);
            oss << std::put_time(CurrenttimeInfo, "%Y-%m-%dT%H:%M:%SZ");
            Response["ReceivedTime"] = oss.str();
        }
    }

    // UUID
    if(DH->Request.JsonRvalue.has("UUID"))
    {
        if(DH->Request.JsonRvalue["UUID"].t() == crow::json::type::String)
        {
            Response["UUID"] = DH->Request.JsonRvalue["UUID"].s();
        }else
        {
            Response["UUID"] = DH->Request.JsonRvalue["UUID"].i();
        }
    }

    if(DH->Modules.Classifier.active)
    {
        for(const auto& keyLabel : DH->ProcessedInputData.ClassifierModuleOutput)
            Response[keyLabel.first] = keyLabel.second;
    }

    // SHOW_IMPORTANTLOG2(crow::json::dump(Response));
    this->OutputKafkaConnection->write({crow::json::dump(Response), DH->ProcessedInputData.MongoID});

    return true;
}
#endif // KAFKAOUTPUT