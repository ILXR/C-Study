#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <queue>
#include <iostream>
#include <chrono>

class ThreadPool
{
public:
    typedef std::function<void()> Task;
    ThreadPool(size_t size) : _size(size), stop(false)
    {
        for (size_t i = 0; i < _size; i++)
        {
            _threads.emplace_back(new std::thread([this]()
                                                  {
                for (;;)
                {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(this->_mutex);
                        while (!this->stop && this->_tasks.empty())
                        {
                            this->_condition.wait(lock);
                        }
                        if (this->stop)
                            return;

                        task = move(this->_tasks.front());
                        this->_tasks.pop();
                    }
                    task();
                } }));
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            stop = true;
        }
        _condition.notify_all();

        for (auto &t : _threads)
        {
            t->join();
        }
    }

    template <typename Func, typename... Args>
    decltype(auto) enqueue(Func &&func, Args &&...args)
    {
        using result_type = typename std::result_of<Func(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<void()>>(std::bind(std::forward<Func>(func),
                                                                           std::forward<Args>(args)...));
        auto res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(_mutex);

            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            _tasks.emplace([task]()
                           { (*task)(); });
        }

        _condition.notify_one();
        return res;
    }

private:
    size_t _size;

    std::queue<Task> _tasks;
    std::vector<std::unique_ptr<std::thread>> _threads;

    std::mutex _mutex;
    std::condition_variable _condition;

    bool stop;
};

int main()
{
    ThreadPool pool(5);
    std::vector<std::future<void>> results;
    for (int i = 0; i < 50; i++)
    {
        results.push_back(pool.enqueue([i]()
                                       {
                                        std::this_thread::sleep_for(std::chrono::seconds(1)); 
                                        std::cout << "thread id:" << std::this_thread::get_id() << ", i:" << i << std::endl; }));
    }
    for (auto &res : results)
    {
        res.wait();
    }
    std::cout << "main thread end" << std::endl;
}