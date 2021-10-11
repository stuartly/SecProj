// set the configuration DataLayoutChecker::Threshold to 1000
struct s{
	int a;
	int b;
}s1;
int main(){
	for(int i=0;i<10000;i++){
		s1.a=10;
	}
	s1.a=s1.b;
	s1.a=10;
	s1.a=10;
	return 0;
}