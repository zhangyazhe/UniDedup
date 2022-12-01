#include "time.hh"

std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::time_t timestamp = tmp.count();
    return timestamp;
}
void printTime(std::time_t timestamp)
{
    std::time_t milli = timestamp + (std::time_t)8*60*60*1000; //转化为东八区北京时间
    auto mTime = std::chrono::milliseconds(milli);
    auto tp = std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds>(mTime);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);
    printf("[%02d:%02d:%02d] ",now->tm_hour,now->tm_min,now->tm_sec);
}
