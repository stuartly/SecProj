#include <memory.h>
#include <string.h>
#include <stdlib.h>

const char *max(const char *str){
	size_t index=0;
	for(size_t i=0;i<strlen(str);i++){
		if(str[index]<str[i]){
			index=i;
		}
	}
	return str+index;
}

int MAX_NODE=100;
int read_configuration_of_max_node(){
	return MAX_NODE;
}
void modify_user_defined_configuration(){
	int config;// it may read from outer file
	// such as config=read_file();
	char end;// a sign
	while(end!='\0'){
		if(config>read_configuration_of_max_node()){
			//...
		}
	}
}