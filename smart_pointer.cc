#include <iostream>
#include <vector>
// C++ smart 指针在使用时必须引用 memory
#include <memory> // unique_ptr
#include <auto_ptr.h>
#include <boost/scoped_ptr.hpp>

using namespace std;

void _auto_ptr()
{
    /**
     * auto_ptr 已经被C++11标准弃用！
     * 在 auto_ptr 中构造函数时浅拷贝，将原有资源转给最新的对象，将自己置为空；
     * auto_ptr 的赋值也是一样，会将自身置空，将最新的指针指向原有空间，所以 auto_ptr 是不允许赋值的。
     *
     * auto_ptr& operator=(auto_ptr& __a) throw()
     * {
     *      reset(__a.release());
     *      return *this;
     * }
     *
     * 当同时有多个 auto_ptr 持有同一个资源指针时，就会发生不可预知的错误（典型的就是访问空指针）；
     * 当然容器中也不能使用，因为容器会设计很多浅拷贝操作，这样会使原始指针失效。
     */
    auto_ptr<int> ptr(new int(1));
    auto_ptr<int> ptr_(ptr); // 此时 ptr 已经指向 null 了
    // cout << *ptr << " " << *ptr_ << endl; // 这里会发生错误，访问了 nullptr
}

void _scope_ptr()
{
    /**
     * scope_ptr 和 auto_ptr 几乎一样，同一时刻只能有一个智能指针来管理某一资源；
     * scope_ptr 把拷贝构造函数和赋值函数都声明为私有的，拒绝了指针的所有权转让，任何人都无权访问被管理的指针，
     * 从而保证了指针的绝对安全。
     *
     * private:
     *      // (只声明，未定义)
     *      scoped_ptr(scoped_ptr const &);
     *      scoped_ptr & operator=(scoped_ptr const &);
     *
     * 概括来说就是：scope_ptr不允许拷贝，赋值，只能在 scope_ptr被声明的作用域内使用，
     * 除了*和->外 scope_ptr也没有定义其他的操作符（不能对 scope_ptr进行++或者--等指针算术操作），
     * 这样就不用考虑浅拷贝的问题了。
     * 同样，因为其不能进行拷贝和赋值，所以不能用作容器的元素。
     */
    boost::scoped_ptr<int> ptr(new int(1));
    // boost::scoped_ptr<int> ptr_(ptr); // 会报错
    // boost::scoped_ptr<int> ptr_ = ptr; // 会报错
}

void _unique_ptr()
{
    /**
     * unique_ptr 基本和 auto_ptr 一致，都是让最后一个指针指向资源，将之前的指针置为空。
     * 只是它禁止了左值引用赋值和拷贝构造，只允许右值引用赋值和构造。
     *
     * // Disable copy from lvalue.
     * unique_ptr(const unique_ptr&) = delete;
     * unique_ptr& operator=(const unique_ptr&) = delete;
     *
     * 因此我们可以通过 std::move 来构建右值引用（或者函数的返回值也是右值），从而实现赋值操作：
     * unique_ptr 也同样不能直接做为容器元素，但可以通过间接作为容器元素（move）
     */
    unique_ptr<int> ptr(new int(1));
    vector<unique_ptr<int>> vec;
    // vec.push_back(ptr); // 发生错误
    vec.push_back(move(ptr)); // 成功执行
}

int main(int argc, char const *argv[])
{
    return 0;
}