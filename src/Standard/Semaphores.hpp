#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore
{
protected:
    std::mutex mtx;
    std::condition_variable cv;
    bool flag = false;
public:
    virtual void open()
    {
        mtx.lock();
        flag = true;
        cv.notify_all();
        mtx.unlock();
    }
    virtual void close()
    {
        mtx.lock();
        flag = false;
        cv.notify_all();
        mtx.unlock();
    }
    virtual void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [this] {return this->flag;});
    }
};

class ImpulseSemaphore : public Semaphore
{
public:
    void wait() override
    {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [this] {return this->flag;});

        flag = false;

        cv.notify_all();
    }

    void close() override {};    
    void open() override {};

    void signal()
    {
        mtx.lock();
        flag = true;
        cv.notify_all();
        mtx.unlock();
    }
};