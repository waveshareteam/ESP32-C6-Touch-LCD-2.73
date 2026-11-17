#pragma once
#include "systems/phone/esp_brookesia_phone_app.hpp"
namespace esp_brookesia::apps {
class TouchTest final: public systems::phone::App {
public:
    static TouchTest *requestInstance();
    ~TouchTest() override { close(); }
protected:
    TouchTest();
    bool init() override { return true; }
    bool run() override;
    bool back() override { return notifyCoreClosed(); }
    bool close() override;
    bool deinit() override { return close(); }
    bool pause() override;
    bool resume() override;
private:
    static void exit(void *context);
};
class PinsTest final: public systems::phone::App {
public:
    static PinsTest *requestInstance();
    ~PinsTest() override { close(); }
protected:
    PinsTest();
    bool init() override { return true; }
    bool run() override;
    bool back() override { return notifyCoreClosed(); }
    bool close() override;
    bool deinit() override { return close(); }
    bool pause() override { return true; }
    bool resume() override { return true; }
};
}
