// disableSumpleVariable =false
// Overlap=true

#include <memory.h>
#include <string.h>
#include <stdlib.h>
char * rte_malloc(size_t size){
	return (char*)malloc(size);
}
void dpdk_buffer(){
	char *test_buffer=NULL;
	// ...
	test_buffer=rte_malloc(1);
}
struct node{
	int value;
	struct node *next;
};
void test_node(){
	struct node *s1=(struct s*)malloc(sizeof(struct node));
	memset(s1,0,sizeof(s1));
	s1->value=10;
	s1->next=NULL;
}
void test_node_alt(){
	struct node *s1=(struct s*)malloc(sizeof(struct node));
	if(s1!=NULL)
		memset(s1,0,sizeof(s1));
	s1->value=10;
	s1->next=NULL;
}