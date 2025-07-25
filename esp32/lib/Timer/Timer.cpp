#include "Timer.h"

// 静态成员初始化
Timer* Timer::instance_ = nullptr;

// 时间间隔常量定义
constexpr uint16_t Timer::TIME_INTERVALS[];

// 全局引用初始化
Timer& timer = Timer::getInstance();

// 构造函数
Timer::Timer() 
    : hwTimer_(nullptr)
    , flagBzw_(false)
    , timCount_(0) {
    
    // 初始化所有时间标志为0
    for (size_t i = 0; i < static_cast<size_t>(TimeFlag::FLAG_COUNT); ++i) {
        timeFlags_[i] = 0;
    }
}

// 析构函数
Timer::~Timer() {
    deinit();
}

// 获取单例实例
Timer& Timer::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new Timer();
    }
    return *instance_;
}

// 初始化硬件定时器
bool Timer::init() {
    if (hwTimer_ != nullptr) {
        Serial.println("⚠️  定时器已经初始化");
        return true;
    }
    
    try {
        // 创建定时器
        hwTimer_ = timerBegin(TIMER_NUM, TIMER_PRESCALER, true);
        if (hwTimer_ == nullptr) {
            Serial.println("❌ 创建定时器失败");
            return false;
        }
        
        // 设置中断回调
        timerAttachInterrupt(hwTimer_, &Timer::timerCallback, true);
        
        // 设置定时器报警值 (1ms)
        timerAlarmWrite(hwTimer_, TIMER_ALARM_US, true);
        
        // 启用定时器
        timerAlarmEnable(hwTimer_);
        
        Serial.println("✅ 定时器初始化成功 (1ms周期)");
        return true;
        
    } catch (...) {
        Serial.println("❌ 定时器初始化异常");
        hwTimer_ = nullptr;
        return false;
    }
}

// 关闭定时器
void Timer::deinit() {
    if (hwTimer_ != nullptr) {
        timerDetachInterrupt(hwTimer_);
        timerAlarmDisable(hwTimer_);
        timerEnd(hwTimer_);
        hwTimer_ = nullptr;
        Serial.println("🔄 定时器已关闭");
    }
}

// 获取时间标志状态
bool Timer::getTimeFlag(TimeFlag flag) const {
    size_t index = static_cast<size_t>(flag);
    if (index >= static_cast<size_t>(TimeFlag::FLAG_COUNT)) {
        return false;
    }
    return timeFlags_[index] != 0;
}

// 清除时间标志
void Timer::clearTimeFlag(TimeFlag flag) {
    size_t index = static_cast<size_t>(flag);
    if (index < static_cast<size_t>(TimeFlag::FLAG_COUNT)) {
        timeFlags_[index] = 0;
    }
}

// 检查并清除时间标志（原子操作）
bool Timer::checkAndClearTimeFlag(TimeFlag flag) {
    size_t index = static_cast<size_t>(flag);
    if (index >= static_cast<size_t>(TimeFlag::FLAG_COUNT)) {
        return false;
    }
    
    if (timeFlags_[index]) {
        timeFlags_[index] = 0;
        return true;
    }
    return false;
}

// LED初始化
bool Timer::ledInit() {

    // 检查引脚是否有效（可选的安全检查）
    if (LED_PIN < 0 || LED_PIN > 39) {  // ESP32引脚范围检查
        Serial.println("✗ LED引脚号无效");
        return false;
    }
    
    // pinMode() 返回 void，直接调用即可
    pinMode(LED_PIN, OUTPUT);
    setLED(false); // 初始状态关闭LED
    
    Serial.print("✓ LED初始化成功，引脚: ");
    Serial.println(LED_PIN);
    return true;
}

// LED闪烁控制
void Timer::ledTick(TimeFlag timeFlag) {
    if (checkAndClearTimeFlag(timeFlag)) {
        toggleLED();
    }
}

// 切换LED状态
void Timer::toggleLED() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// 设置LED状态
void Timer::setLED(bool state) {
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

// 重置定时器
void Timer::reset() {
    timCount_ = 0;
    flagBzw_ = false;
    
    // 清除所有时间标志
    for (size_t i = 0; i < static_cast<size_t>(TimeFlag::FLAG_COUNT); ++i) {
        timeFlags_[i] = 0;
    }
}

// 静态中断回调函数
void IRAM_ATTR Timer::timerCallback() {
    if (instance_ != nullptr) {
        instance_->flagBzw_ = true;
        instance_->timCount_++;
        instance_->updateTimeFlags();
    }
}

// 更新时间标志（在中断中调用）
void Timer::updateTimeFlags() {
    // 1ms 标志始终设置
    timeFlags_[static_cast<size_t>(TimeFlag::FLAG_1MS)] = 1;
    
    // 其他时间标志根据计数判断
    const size_t flagCount = static_cast<size_t>(TimeFlag::FLAG_COUNT) - 1; // 除了3s标志
    
    for (size_t i = 1; i < flagCount; ++i) {
        if (timCount_ % TIME_INTERVALS[i] == 0) {
            timeFlags_[i] = 1;
        }
    }
    
    // 特殊处理3s标志（重置计数器）
    if (timCount_ >= TIME_INTERVALS[static_cast<size_t>(TimeFlag::FLAG_3S)]) {
        timeFlags_[static_cast<size_t>(TimeFlag::FLAG_3S)] = 1;
        timCount_ = 0;
    }
}
