#include <mutex>
int global_int = 0;

class C
{
public:
    void f(int x, int y)
    {
        x++;
    }
};

C h;

void func()
{
    int sum = 0;
    for (int i = 0; i < 10000; i++)
    {
        sum += i;
    }
}

void g(int x)
{
    x++;
}
int Reentrant(int a) { return a * a; }

void test1()
{
    std::mutex mtx1;
    mtx1.lock();
    Reentrant(5);
    mtx1.unlock();
}

int g_test = 1;
int non_Reentrant()
{
    g_test++;
    return g_test;
}

void test2()
{
    std::mutex mtx1;
    mtx1.lock();
    non_Reentrant();
    mtx1.unlock();
}
int global_variable = 0;
void test3()
{
    std::mutex mtx2;
    mtx2.lock();
    for (int i = 0; i < 10; i++)
    {
        global_variable++;
    }
    mtx2.unlock();
}

int main()
{
    test1();
    test2();
    test3();

    std::mutex mtx1;
    std::mutex mtx2;
    std::mutex mtx3;
    std::mutex mtx4;
    int x = 0;
    int a = 1;
    mtx1.lock();
    func();
    int b = 2;
    x++;
    x = a + b;
    mtx1.unlock();

    for (;;)
    {
        x++;
    }

    func();
    x++;

    mtx2.lock();
    x--;
    func();
    mtx2.unlock();

    mtx1.lock();
    mtx2.lock();

    g(global_int);
    C d;
    d.f(global_int, x);
    d.f(x, global_int);
    h.f(x, x);
    h.f(global_int, x);

    func();
    mtx2.unlock();
    x *= 1;
    mtx3.lock();
    func();
    mtx4.lock();
    func();
    mtx4.unlock();
    func();
    mtx3.unlock();
    x /= 1;
    mtx1.unlock();
    C c;
    c.f(x, a);

    return 0;
}