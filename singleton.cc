#include <iostream>
#include <memory>

using namespace std;

/**
 * C++ 11 标准做法，c++11 局部静态变量已经是线性安全了
 * 通过局部 static 变量只能初始化一次的特性，实现单例
 */
class Singleton
{
public:
    static Singleton &getInstance()
    {
        static Singleton instance;
        return instance;
    }
    Singleton(Singleton &) = delete;
    Singleton &operator=(Singleton &) = delete;

private:
    Singleton() = default;
};

/**
 * 普通懒汉式（Lazy Singleton）
 * 在第一次使用时才进行初始化，这叫做延时初始化
 */
class NormalLazy
{
public:
    NormalLazy *getInstance()
    {
        if (instance == nullptr)
        {
            instance = new NormalLazy();
        }
        return instance;
    }
    NormalLazy(NormalLazy &) = delete;
    NormalLazy &operator=(NormalLazy &) = delete;

private:
    NormalLazy() = default;

private:
    static NormalLazy *instance;
};
NormalLazy *NormalLazy::instance = nullptr;

/**
 * 首先，上面的懒汉式单例模式存在内存泄漏的问题。有两种解决办法：
 *      使用独占式智能指针std::unique_ptr;
 *      使用静态的嵌套类对象；
 */

// 使用智能指针
class LazyUnique
{
public:
    LazyUnique *getInstance()
    {
        if (instance.get() == nullptr)
        {
            instance.reset(new LazyUnique());
        }
        return instance.get();
    }
    LazyUnique(LazyUnique &) = delete;
    LazyUnique &operator=(LazyUnique &) = delete;

private:
    LazyUnique() = default;

private:
    static unique_ptr<LazyUnique> instance;
};
unique_ptr<LazyUnique> LazyUnique::instance{nullptr};

/** 静态嵌套类
 * 在程序运行结束时，系统会调用静态成员delector的析构函数，该析构函数会删除单例的唯一实例。
 * 使用这种方法释放单例对象有以下特征：
 *      在单例类内部定义专有的嵌套类。
 *      在单例类内定义私有的专门用于释放的静态成员。
 *      利用程序在结束时析构全局变量的特性，选择最终的释放时机。
 */
class LazyEmbeded
{
public:
    LazyEmbeded *getInstance()
    {
        if (instance == nullptr)
        {
            instance = new LazyEmbeded();
        }
        return instance;
    }
    LazyEmbeded(LazyEmbeded &) = delete;
    LazyEmbeded &operator=(LazyEmbeded &) = delete;

private:
    LazyEmbeded() = default;
    struct Deletor
    {
        ~Deletor()
        {
            if (instance != nullptr)
            {
                delete instance;
                instance = nullptr;
            }
        }
    };

private:
    static LazyEmbeded *instance;
    static Deletor deletor;
};
LazyEmbeded *LazyEmbeded::instance = nullptr;
LazyEmbeded::Deletor LazyEmbeded::deletor;


int main()
{
    return 0;
}