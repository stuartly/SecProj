/*检测某个结构体在放进某个容器前未初始化*/
#include <vector>
#include <queue>
using namespace std;
struct A
{
  int member_1;
  int member_2;
};
void func(A a)
{
  vector<A> v;
  v.push_back(a);
  queue<A> q;
  q.push(a);
}
int main()
{
  A a;
  a.member_1=2;
//   a.member_2=0;
  func(a);
  vector<A> v;
  v.push_back(a);//a必须每个成员变量都有初始化或者有初始化列表才不报错
  queue<A> q;
  q.push(a);
  return 0;
}

/*配置depth>=3可检测到初始化,否则在func函数内报错*/
/*#include <stdlib.h>
using namespace std;
void use(int a);
struct A{
  int val;
};
 void func(A *a){
   use(a->val);
 }
 void func_1(A *a){
   func(a);
 }
  void func_2(A *a){
   func_1(a);
 }
int main(){
  A a;
  a.val=2;
  func_2(&a);
  return 0;
}*/

