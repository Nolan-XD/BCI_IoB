/*
 * @Author: Nolan-XD haopengliuu@163.com
 * @Date: 2025-07-24 17:55:42
 * @LastEditors: Nolan-XD haopengliuu@163.com
 * @LastEditTime: 2025-07-25 09:55:06
 * @FilePath: \BCI_IoB_main\esp32\lib\timer\timer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <Arduino.h>

class Timer {
public:
    // 时间标志枚举
    enum class TimeFlag : uint8_t {
        FLAG_1MS = 0,
        FLAG_2MS,
        FLAG_5MS,
        FLAG_10MS,
        FLAG_18MS,
        FLAG_20MS,
        FLAG_50MS,
        FLAG_100MS,
        FLAG_200MS,
        FLAG_500MS,
        FLAG_1S,
        FLAG_2S,
        FLAG_3S,
        FLAG_COUNT  // 总数量
    };
    // LED引脚常量
    static constexpr uint8_t LED_PIN = 2;
    
    // 定时器参数常量
    static constexpr uint8_t TIMER_NUM = 0;
    static constexpr uint16_t TIMER_PRESCALER = 80;  // 80MHz -> 1MHz
    static constexpr uint32_t TIMER_ALARM_US = 1000; // 1ms
private:
    // 静态实例指针（单例模式）
    static Timer* instance_;
    
    // 定时器句柄
    hw_timer_t* hwTimer_;
    
    // 定时器变量
    volatile bool flagBzw_;
    volatile uint16_t timCount_;
    volatile uint8_t timeFlags_[static_cast<size_t>(TimeFlag::FLAG_COUNT)];
    
    // 时间间隔配置
    static constexpr uint16_t TIME_INTERVALS[] = {
        1,    // 1ms
        2,    // 2ms
        5,    // 5ms
        10,   // 10ms
        18,   // 18ms
        20,   // 20ms
        50,   // 50ms
        100,  // 100ms
        200,  // 200ms
        500,  // 500ms
        1000, // 1s
        2000, // 2s
        3000  // 3s
    };
public:
    // 构造函数和析构函数
    Timer();
    ~Timer();
    
    // 禁止拷贝构造和赋值操作（单例模式）
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    
    // 获取单例实例
    static Timer& getInstance();
    
    // 定时器控制
    bool init();
    void deinit();
    bool isInitialized() const { return hwTimer_ != nullptr; }
    
    // 时间标志操作
    bool getTimeFlag(TimeFlag flag) const;
    void clearTimeFlag(TimeFlag flag);
    bool checkAndClearTimeFlag(TimeFlag flag);
    
    // LED控制
    bool ledInit();
    void ledTick(TimeFlag timeFlag);
    void toggleLED();
    void setLED(bool state);
    
    // 获取当前计数
    uint16_t getTimCount() const { return timCount_; }
    bool getBzwFlag() const { return flagBzw_; }
    void clearBzwFlag() { flagBzw_ = false; }
    
    // 重置定时器
    void reset();
private:
    // 静态中断回调函数
    static void IRAM_ATTR timerCallback();
    
    // 内部更新函数
    void updateTimeFlags();
};
// 便于使用的全局引用
extern Timer& timer;
