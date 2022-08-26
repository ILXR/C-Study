#include <iostream>
using namespace std;

#pragma region 相关知识
/** 如何查看虚表内容？
 *
 * 使用 g++ -fdump-lang-class vtable.cc 来生成虚表布局文件 vtable.cc.*.class
 * 使用 g++ vtable.cc -o vtable -g 来生成可调式文件 vtable
 * 使用 gdb 中的 i vtbl object 来查看 object 对象的虚表内容
 * 使用 gdb 查看一个对象的内存布局：
 *      set p pretty // 缩进输出
 *      set p object
 *      set p vtbl
 *      p obj
 */

/** 关于 typeinfo 的相等性判断
 *
 * typeinfo 指针指向用于 RTTI 的 typeinfo 对象，它总是存在的。 给定类的每个vtable中的所有typeinfo
 * 指针都必须指向相同的 typeinfo 对象。
 * typeinfo 相等性的正确实现是检查指针相等性，但指向不完整类型的指针（直接或间接）除外。
 * typeinfo 指针在多态类的场景下是有效指针，对于非多态类为零。
 */

/** Thunk
 * 所谓thunk是一小段汇编代码，用来
 *      （1）以适当的offset值调整this指针
 *      （2)跳到virtual function去
 * 这样就实现了虚表内，函数被覆盖时，通过偏移量来调用最终的函数实现
 */

/** 虚表中存在两个析构函数 complete object destructor 和 deleting destructor
 *
 * complete object destructor：除了具备base object destructor的功能外，
 *                             还会调用该类的虚拟基类的析构函数
 * deleting destructor：先调用 complete object destructor 来析构，再调用适当的operator delete函数释放内存
 * base object destructor：不涉及多态，因此不存储在虚表中，调用该类的非静态数据成员和非虚拟直接基类运行析构函数
 *
 * gcc编译器会在编译时将基类的虚析构函数的地址安插在complete object destructor中，并在派生类的vtbale中的
 * 一个slot中安插上complete object destructor的地址。
 * 当通过基类的指针delete派生类时，gcc编译器会调整this指针，指向派生类，然后通过该this指针调用
 * 相应的deleting destructor函数。
 */

/** 使用父指针访问子类独有的函数
 *
 * 任何妄图使用父类指针调用子类中的未覆盖父类的成员函数的行为都会被编译器视为非法，所以，
 * 这样的程序根本无法编译通过。
 * 但在运行时，我们可以通过指针的方式访问虚函数表来达到违反C++语义的行为。
 */

/** 构造函数 / 析构函数
 * 当程序执行一个构造函数时，会进行如下操作：
 *      1. 依次调用父类的构造函数，如果没有指定则调用父类的默认构造函数
 *      2. 如果有虚函数，设置虚函数指针
 *      3. 根据初始化成员列表对成员变量进行初始化，如果没有指定则使用其默认值或initialize list中的参数进行初始化
 *      4. 执行构造函数中的代码
 *
 * 析构函数和构造函数类似，但是执行顺序是相反的。在父类的的析构函数中调用虚函数，因为此时虚指针已经指向父类的虚表，
 * 所以并不会调用到子类的虚函数：
 *      1. 执行析构函数中的代码
 *      2. 执行成员变量的析构函数
 *      3. 设置虚指针为父类的虚指针
 *      4. 依次调用父类的析构函数
 */
#pragma endregion

#pragma region 无继承 / 有虚函数
/*
Vtable for Base
Base::_ZTV4Base: 5 entries
0     (int (*)(...))0   offset to top, 含义见下方
8     (int (*)(...))(& _ZTI4Base)   指向 Base 的 type_info，一般只存储 char* name 这一个变量
16    (int (*)(...))Base::~Base
24    (int (*)(...))Base::~Base
32    (int (*)(...))Base::f
40    (int (*)(...))Base::g
48    (int (*)(...))Base::h

Class Base
   size=16 align=8
   base size=12 base align=8

** offset_to_top，表示该类虚表指针距离对象顶部地址的偏移量，这里是0，因为对象内存的第一项就是虚表指针，
只有存在多重继承才不为0。
在向上动态转换到实际类型时，让this指针加上这个偏移量即可得到实际类型的地址。

实际对象存储的虚表指针是 vtable_ptr + 16，也就是跳过了 vcall offset 和 type_info ptr 部分的内容，直接指向第一个
虚函数的地址。
 */
class Base
{
public:
    int a = 1;
    virtual ~Base() {}
    virtual void f() { cout << "Base::f" << endl; }
    virtual void g() { cout << "Base::g" << endl; }
    virtual void h() { cout << "Base::h" << endl; }
};
void vtable_model()
{
    typedef void (*Fun)(void);
    Base b;
    cout << "对象地址：" << &b << endl;
    long *vtb_ptr = (long *)*(long *)(&b);
    cout << "虚函数表地址：" << vtb_ptr << endl;
    cout << "vcall offset：" << *(vtb_ptr - 2) << endl;
    type_info *info = (type_info *)*(vtb_ptr - 1);
    cout << "typeinfo 地址" << info << endl;
    cout << "typeinfo name：" << info->name() << endl;
    // Invoke(调用) the first virtual function
    Fun pFun = (Fun)*vtb_ptr;
    pFun();
}
#pragma endregion

#pragma region 单继承 / 无虚函数覆盖
class Derived_no_override : public Base
{
public:
    int a1;
    virtual void f1() { cout << "Derived::f1" << endl; }
    virtual void g1() { cout << "Derived::g1" << endl; }
    virtual void h1() { cout << "Derived::h1" << endl; }
};
/* 有继承 / 无虚函数覆盖 - 虚表内存分布
1）虚函数按照其声明顺序放于表中。
2）父类的虚函数在子类的虚函数前面。

Vtable for Derived_no_override
Derived_no_override::_ZTV19Derived_no_override: 10 entries
0     (int (*)(...))0
8     (int (*)(...))(& _ZTI19Derived_no_override)
16    (int (*)(...))Derived_no_override::~Derived_no_override
24    (int (*)(...))Derived_no_override::~Derived_no_override
32    (int (*)(...))Base::f
40    (int (*)(...))Base::g
48    (int (*)(...))Base::h
56    (int (*)(...))Derived_no_override::f1
64    (int (*)(...))Derived_no_override::g1
72    (int (*)(...))Derived_no_override::h1

Class Derived_no_override
   size=16 align=8
   base size=16 base align=8
Derived_no_override (0x0x7fad15f0fa90) 0
    vptr=((& Derived_no_override::_ZTV19Derived_no_override) + 16)
  Base (0x0x7fad15f07ea0) 0
      primary-for Derived_no_override (0x0x7fad15f0fa90)
*/
#pragma endregion

#pragma region 单继承 / 有虚函数覆盖
class Derived_override : public Base
{
public:
    int b = 1;
    virtual void f() { cout << "Derived::f" << endl; }
    virtual void g() { cout << "Derived::g" << endl; }
    virtual void h1() { cout << "Derived::h1" << endl; }
};
/*
1）覆盖的 f() 和 g() 函数被放到了虚表中原来父类虚函数的位置。
2）没有被覆盖的函数依旧。

Vtable for Derived_override
Derived_override::_ZTV16Derived_override: 8 entries
0     (int (*)(...))0
8     (int (*)(...))(& _ZTI16Derived_override)
16    (int (*)(...))Derived_override::~Derived_override
24    (int (*)(...))Derived_override::~Derived_override
32    (int (*)(...))Derived_override::f
40    (int (*)(...))Derived_override::g
48    (int (*)(...))Base::h
56    (int (*)(...))Derived_override::h1

Class Derived_override
   size=16 align=8
   base size=16 base align=8
Derived_override (0x0x7fad15f0fbc8) 0
    vptr=((& Derived_override::_ZTV16Derived_override) + 16)
  Base (0x0x7fad15f4d0c0) 0
      primary-for Derived_override (0x0x7fad15f0fbc8)
*/
#pragma endregion

#pragma region 多继承 / 无虚函数覆盖
class Base1
{
public:
    int b1;
    virtual ~Base1() {}
    virtual void f() { cout << "Base1::f" << endl; }
    virtual void g() { cout << "Base1::g" << endl; }
    virtual void h() { cout << "Base1::h" << endl; }
};
class Base2
{
public:
    int b2;
    virtual ~Base2() {}
    virtual void f() { cout << "Base2::f" << endl; }
    virtual void g() { cout << "Base2::g" << endl; }
    virtual void h() { cout << "Base2::h" << endl; }
};
class Base3
{
public:
    int b3;
    virtual ~Base3() {}
    virtual void f() { cout << "Base3::f" << endl; }
    virtual void g() { cout << "Base3::g" << endl; }
    virtual void h() { cout << "Base3::h" << endl; }
};
class Derived_multi_no_override : Base1, Base2, Base3
{
public:
    int d;
    virtual ~Derived_multi_no_override() {}
    virtual void f1() { cout << "Derived::f" << endl; }
    virtual void g1() { cout << "Derived::g" << endl; }
    virtual void h1() { cout << "Derived::h" << endl; }
};
/*
使用 gdb 查看对象内存布局：
(Derived_multi_no_override) {
  <Base1> = {
    _vptr.Base1 = 0x555555558b40 <vtable for Derived_multi_no_override+16>,
    b1 = 1431659740
  },
  <Base2> = {
    _vptr.Base2 = 0x555555558b90 <vtable for Derived_multi_no_override+96>,
    b2 = 1431661485
  },
  <Base3> = {
    _vptr.Base3 = 0x555555558bc8 <vtable for Derived_multi_no_override+152>,
    b3 = 1431661408
  },
  members of Derived_multi_no_override:
  d = 21845
}

再查看虚表布局：
Vtable for Derived_multi_no_override
Derived_multi_no_override::_ZTV25Derived_multi_no_override: 24 entries
0     (int (*)(...))0
8     (int (*)(...))(& _ZTI25Derived_multi_no_override)
16    (int (*)(...))Derived_multi_no_override::~Derived_multi_no_override
24    (int (*)(...))Derived_multi_no_override::~Derived_multi_no_override
32    (int (*)(...))Base1::f
40    (int (*)(...))Base1::g
48    (int (*)(...))Base1::h
56    (int (*)(...))Derived_multi_no_override::f1
64    (int (*)(...))Derived_multi_no_override::g1
72    (int (*)(...))Derived_multi_no_override::h1
80    (int (*)(...))-16
88    (int (*)(...))(& _ZTI25Derived_multi_no_override)
96    (int (*)(...))Derived_multi_no_override::_ZThn16_N25Derived_multi_no_overrideD1Ev
104   (int (*)(...))Derived_multi_no_override::_ZThn16_N25Derived_multi_no_overrideD0Ev
112   (int (*)(...))Base2::f
120   (int (*)(...))Base2::g
128   (int (*)(...))Base2::h
136   (int (*)(...))-32
144   (int (*)(...))(& _ZTI25Derived_multi_no_override)
152   (int (*)(...))Derived_multi_no_override::_ZThn32_N25Derived_multi_no_overrideD1Ev
160   (int (*)(...))Derived_multi_no_override::_ZThn32_N25Derived_multi_no_overrideD0Ev
168   (int (*)(...))Base3::f
176   (int (*)(...))Base3::g
184   (int (*)(...))Base3::h

Class Derived_multi_no_override
   size=48 align=8
   base size=48 base align=8
Derived_multi_no_override (0x0x7f99fe2c1d20) 0
    vptr=((& Derived_multi_no_override::_ZTV25Derived_multi_no_override) + 16)
  Base1 (0x0x7f99fe2f3de0) 0
      primary-for Derived_multi_no_override (0x0x7f99fe2c1d20)
  Base2 (0x0x7f99fe2f3e40) 16
      vptr=((& Derived_multi_no_override::_ZTV25Derived_multi_no_override) + 96)
  Base3 (0x0x7f99fe2f3ea0) 32
      vptr=((& Derived_multi_no_override::_ZTV25Derived_multi_no_override) + 152)

可以看到：
1） 子类对象中存了多个父类的续表指针，每个父类都有自己的虚表，但实际上是指向一整张虚表中的不同位置
2） 子类的成员函数被放到了第一个父类的表中（所谓的第一个父类是按照声明顺序来判断的）
3)  除了第一个基类以外，其他基类的虚表还存有 non-virtual thunk to Derived_multi_no_override::~Derived_multi_no_override()
    这是一个偏移量，C++ 中基类对象指针调用派生类对象时,编译器通过thunk技术来实现每次参数调用和参数返回this地址的调整

** non-virtual thunk 在多态调用时，将指针或者引用调整到其持有的实际对象首地址
*/
#pragma endregion

#pragma region 多继承 / 有虚函数覆盖
class Derived_multi_override : Base1, Base2, Base3
{
public:
    int d;
    virtual ~Derived_multi_override() {}
    virtual void f() { cout << "Derived::f" << endl; }
    virtual void g1() { cout << "Derived::g1" << endl; }
    virtual void h() { cout << "Derived::h" << endl; }
};
/*
该派生类的内存结构如下：
(Derived_multi_override) {
  <Base1> = {
    _vptr.Base1 = 0x555555558b50 <vtable for Derived_multi_override+16>,
    b1 = 1431659740
  },
  <Base2> = {
    _vptr.Base2 = 0x555555558b90 <vtable for Derived_multi_override+80>,
    b2 = 1431661517
  },
  <Base3> = {
    _vptr.Base3 = 0x555555558bc8 <vtable for Derived_multi_override+136>,
    b3 = 1431661440
  },
  members of Derived_multi_override:
  d = 21845
}
可见，与非覆盖的布局一致，因此两者区别只有虚表实际内容不同

该派生类的虚表结构如下：
Vtable for Derived_multi_override
Derived_multi_override::_ZTV22Derived_multi_override: 22 entries
0     (int (*)(...))0
8     (int (*)(...))(& _ZTI22Derived_multi_override)
16    (int (*)(...))Derived_multi_override::~Derived_multi_override
24    (int (*)(...))Derived_multi_override::~Derived_multi_override
32    (int (*)(...))Derived_multi_override::f
40    (int (*)(...))Base1::g
48    (int (*)(...))Derived_multi_override::h
56    (int (*)(...))Derived_multi_override::g1
64    (int (*)(...))-16
72    (int (*)(...))(& _ZTI22Derived_multi_override)
80    (int (*)(...))Derived_multi_override::_ZThn16_N22Derived_multi_overrideD1Ev
88    (int (*)(...))Derived_multi_override::_ZThn16_N22Derived_multi_overrideD0Ev
96    (int (*)(...))Derived_multi_override::_ZThn16_N22Derived_multi_override1fEv
104   (int (*)(...))Base2::g
112   (int (*)(...))Derived_multi_override::_ZThn16_N22Derived_multi_override1hEv
120   (int (*)(...))-32
128   (int (*)(...))(& _ZTI22Derived_multi_override)
136   (int (*)(...))Derived_multi_override::_ZThn32_N22Derived_multi_overrideD1Ev
144   (int (*)(...))Derived_multi_override::_ZThn32_N22Derived_multi_overrideD0Ev
152   (int (*)(...))Derived_multi_override::_ZThn32_N22Derived_multi_override1fEv
160   (int (*)(...))Base3::g
168   (int (*)(...))Derived_multi_override::_ZThn32_N22Derived_multi_override1hEv

Class Derived_multi_override
   size=48 align=8
   base size=48 base align=8
Derived_multi_override (0x0x7f2dac0820f0) 0
    vptr=((& Derived_multi_override::_ZTV22Derived_multi_override) + 16)
  Base1 (0x0x7f2dac07f240) 0
      primary-for Derived_multi_override (0x0x7f2dac0820f0)
  Base2 (0x0x7f2dac07f2a0) 16
      vptr=((& Derived_multi_override::_ZTV22Derived_multi_override) + 80)
  Base3 (0x0x7f2dac07f300) 32
      vptr=((& Derived_multi_override::_ZTV22Derived_multi_override) + 136)

或者，采用gdb中更直观的方式来查看：
vtable for 'Derived_multi_override' @ 0x555555558b50 (subobject @ 0x7fffffffdb80):
[0]: 0x55555555691a <Derived_multi_override::~Derived_multi_override()>
[1]: 0x55555555699a <Derived_multi_override::~Derived_multi_override()>
[2]: 0x5555555569de <Derived_multi_override::f()>
[3]: 0x55555555669e <Base1::g()>
[4]: 0x555555556a6a <Derived_multi_override::h()>
[5]: 0x555555556a2e <Derived_multi_override::g1()>

vtable for 'Base2' @ 0x555555558b90 (subobject @ 0x7fffffffdb90):
[0]: 0x555555556985 <non-virtual thunk to Derived_multi_override::~Derived_multi_override()>
[1]: 0x5555555569d3 <non-virtual thunk to Derived_multi_override::~Derived_multi_override()>
[2]: 0x555555556a23 <non-virtual thunk to Derived_multi_override::f()>
[3]: 0x5555555567a0 <Base2::g()>
[4]: 0x555555556aaf <non-virtual thunk to Derived_multi_override::h()>

vtable for 'Base3' @ 0x555555558bc8 (subobject @ 0x7fffffffdba0):
[0]: 0x55555555698f <non-virtual thunk to Derived_multi_override::~Derived_multi_override()>
[1]: 0x5555555569c9 <non-virtual thunk to Derived_multi_override::~Derived_multi_override()>
[2]: 0x555555556a19 <non-virtual thunk to Derived_multi_override::f()>
[3]: 0x5555555568a2 <Base3::g()>
[4]: 0x555555556aa5 <non-virtual thunk to Derived_multi_override::h()>

可以发现，三个父类虚函数表中的f()和h()的位置被替换成了子类的函数指针。
这样，我们就可以任一静态类型的父类来指向子类，并调用子类的f()了
*/
#pragma endregion

#pragma region 菱形继承 / 使用 virtual
class VBase1 : virtual Base
{
public:
    int vb1;
    virtual ~VBase1() {}
    virtual void f() { cout << "vBase1::f" << endl; }
    virtual void f1() { cout << "vBase1::f1" << endl; }
    virtual void g() { cout << "vBase1::g" << endl; }
};
class VBase2 : virtual Base
{
public:
    int vb2;
    virtual ~VBase2() {}
    virtual void f() { cout << "vBase2::f" << endl; }
    virtual void f2() { cout << "vBase2::f2" << endl; }
};
class VDerived : VBase1, VBase2
{
public:
    int vd;
    virtual ~VDerived() {}
    virtual void f() { cout << "VDerived::f" << endl; }
    virtual void fd() { cout << "VDerived::fd" << endl; }
};
/*
出现虚拟继承时，虚表中多了一个字段：vcall offset / vbase offset
vcall offset全称为(virtual call offset), 也即 虚拟调用偏移。当一个class存在虚基类时，gcc编译器便会在vtable中
安插相应的vcall offset。 它的意思是虚基类相对于当前this指针的偏移，针对在虚基类或者虚基类的基类中声明的
virtual function，为了通过虚基类调用virtual function所执行的this指针调整。

在菱形虚拟继承中，除了虚表外，还新增了 两个construction vtable和一个VTT；
    construction vtable 是在构造父类子对象的时候用的
    VTT 的意思是virtual table table，意思是虚表的表，里面存的是虚表的入口地址

其内存布局如下：
(VDerived) {
  <VBase1> = {
    <Base> = {
      _vptr.Base = 0x555555558b00 <vtable for VDerived+176>,
      a = 1
    },
    members of VBase1:
    _vptr.VBase1 = 0x555555558a68 <vtable for VDerived+24>,
    vb1 = 1431659740
  },
  <VBase2> = {
    members of VBase2:
    _vptr.VBase2 = 0x555555558ab0 <vtable for VDerived+96>,
    vb2 = 1431661293
  },
  members of VDerived:
  vd = 21845
}

其虚表布局如下：
vtable for 'VDerived' @ 0x555555558a68 (subobject @ 0x7fffffffdb80):
[0]: 0x555555556814 <VDerived::~VDerived()>
[1]: 0x5555555568b4 <VDerived::~VDerived()>
[2]: 0x5555555568fa <VDerived::f()>
[3]: 0x5555555566c8 <VBase1::f1()>
[4]: 0x555555556704 <VBase1::g()>
[5]: 0x55555555694c <VDerived::fd()>

vtable for 'VBase2' @ 0x555555558ab0 (subobject @ 0x7fffffffdb90):
[0]: 0x5555555568a6 <non-virtual thunk to VDerived::~VDerived()>
[1]: 0x5555555568f0 <non-virtual thunk to VDerived::~VDerived()>
[2]: 0x555555556942 <non-virtual thunk to VDerived::f()>
[3]: 0x5555555567d8 <VBase2::f2()>

vtable for 'Base' @ 0x555555558b00 (subobject @ 0x7fffffffdba0):
[0]: 0x555555556896 <virtual thunk to VDerived::~VDerived()>
[1]: 0x5555555568e3 <virtual thunk to VDerived::~VDerived()>
[2]: 0x555555556935 <virtual thunk to VDerived::f()>
[3]: 0x55555555673f <virtual thunk to VBase1::g()>
[4]: 0x5555555565d8 <Base::h()>

vtable for VDerived:
        .quad   32 -> vcall offset / vbase offset
        .quad   0 -> offset to top
        .quad   typeinfo for VDerived
        .quad   VDerived::~VDerived() [complete object destructor]
        .quad   VDerived::~VDerived() [deleting destructor]
        .quad   VDerived::f()
        .quad   VBase1::f1()
        .quad   VBase1::g()
        .quad   VDerived::fd()
        .quad   16
        .quad   -16
        .quad   typeinfo for VDerived
        .quad   non-virtual thunk to VDerived::~VDerived() [complete object destructor]
        .quad   non-virtual thunk to VDerived::~VDerived() [deleting destructor]
        .quad   non-virtual thunk to VDerived::f()
        .quad   VBase2::f2()
        .quad   0
        .quad   -32
        .quad   -32
        .quad   -32
        .quad   -32
        .quad   typeinfo for VDerived
        .quad   virtual thunk to VDerived::~VDerived() [complete object destructor]
        .quad   virtual thunk to VDerived::~VDerived() [deleting destructor]
        .quad   virtual thunk to VDerived::f()
        .quad   virtual thunk to VBase1::g()
        .quad   Base::h()

可见：
    在内存布局中，先是第一个继承的父类，然后是其他父类，然后是派生类，最后是超类，
*/
#pragma endregion

int main(int argc, char const *argv[])
{
    VDerived d;
    return 0;
}