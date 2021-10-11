struct A {
  int a[4];
};

int main() {
  // 单流水线
  int a[100]{0};
  int sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += a[i];
  }
  int i = 0;
  while (i < 100) {
    sum += a[i];
    ++i;
  }
  i = 0;
  do {
    sum += a[i];
    ++i;
  } while (i < 100);
  // 访问内存操作向前移
  int x = 0, y = 0;
  int z = 0;
  int p[1024]{0};
  for (i = 0; i < 1024; ++i) {
    x += 1;
    y += i;
    z = p[i];
  }
  return 0;
}