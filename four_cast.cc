#include <iostream>

using namespace std;

class base
{
private:
    // 可以通过将 new 运算符重载为私有成员，从而使得 base 只能在栈上分配空间
    // void *operator new(size_t size) {}
    int val;

public:
    void setval(int v) { val = v; }
};

class derived : public base
{
};

/**
 * reinterpret_cast 其实就是强制转换
 * 目标类型必须是一个指针、引用、算术类型、函数指针或者成员指针。它可以用于类型之间进行强制转换。
 * 很好理解，两个不同的实例化对象肯定是不能直接转化的，但是指针是可以的。
 */
void reinterpret_ca()
{
    void *p;
    // 以下两句是等效的
    base *b = reinterpret_cast<base *>(p);
    b = (base *)p;
}

/** const_cast<type_id> (expression)
 * 该运算符用来修改类型的const或volatile属性。除了const 或volatile修饰之外， type_id和expression的类型是一样的。
 * 拥有 volatile 属性的变量，每次读取必须直接从内存读，而不是寄存器，可以用在多线程同步场景下。
 * 用法如下：
 *      常量指针被转化成非常量的指针，并且仍然指向原来的对象
 *      常量引用被转换成非常量的引用，并且仍然指向原来的对象
 *      const_cast一般用于修改底指针。如const char *p形式
 * 如下例子可知，const_cast 可以转换顶层、底层const
 */
void const_ca()
{
    const base *cb = new base();        // 底层cosnt，常量指针，指向常量的指针
    base *const ucb = new base();       // 顶层cosnt，指针常量，类型为指针的常量
    const base *const acb = new base(); //

    base *b = const_cast<base *>(cb);
    b->setval(1);
    b = const_cast<base *>(ucb);
    b->setval(2);
    b = nullptr;
    b = const_cast<base *>(acb);
    b->setval(3);
    b = nullptr;

    delete cb, ucb, acb, b;
}

/** static cast 用法
 * 用于类层次结构中基类（父类）和派生类（子类）之间指针或引用引用的转换
 *      进行上行转换（把派生类的指针或引用转换成基类表示）是安全的
 *      进行下行转换（把基类指针或引用转换成派生类表示）时，由于没有动态类型检查，所以是不安全的
 * 用于基本数据类型之间的转换，如把int转换成char，把int转换成enum。这种转换的安全性也要开发人员来保证。
 * 把空指针转换成目标类型的空指针
 * 把任何类型的表达式转换成void类型
 */
void static_ca()
{
    int a = 20;
    float b = static_cast<float>(a);
    cout << "Static cast from int to float" << endl;
    cout << a << " " << b << endl;
}

/** dynamic_cast 用法
 * 有类型检查，基类向派生类转换比较安全，但是派生类向基类转换则不太安全
 * dynamic_cast 运算符可以在执行期决定真正的类型，也就是说expression必须是多态类型。
 * 如果下行转换是安全的（也就说，如果基类指针或者引用确实指向一个派生类对象）这个运算符会传回适当转型过的指针。
 * 如果下行转换不安全，这个运算符会传回空指针（也就是说，基类指针或者引用没有指向一个派生类对象）
 *
 * 在类层次间进行上行转换时，dynamic_cast和static_cast的效果是一样的
 * 在进行下行转换时，dynamic_cast具有类型检查的功能，比static_cast更安全
 */
void dynamic_ca()
{
    base *b = new base();
    derived *d = new derived();
    b = dynamic_cast<base *>(d);   // 向上可以成功转换
    d = static_cast<derived *>(b); // 可以通过编译并且能够执行，但是这是错误并且不安全的
    // d = dynamic_cast<derived *>(b); // 编译时会进行检查并报错，如果编译无法检查，就会在错误时返回空指针

    delete b, d;
}

int main(int argc, char const *argv[])
{
    const_ca();
    return 0;
}
