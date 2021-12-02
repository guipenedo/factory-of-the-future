#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {	
	if( argc < 2 ){	
		printf("\n Please input the arguments in the format:  ./machine_learning no_of_factories\n");
		exit(EXIT_FAILURE);
	}
	struct stat st = {0}; //check if directory exists
	char f[20] = "./factory/factory"; 
	char temp[50];
	int arg = strtol(argv[1], NULL, 10);
	char source_code[10] = "./src/*";
	char data[10] = "./data/*";
	char command[100];
	for(int i=1; i <= arg; i++){
		sprintf(source_code, "./src/*");
		sprintf(data, "./data/*");
		sprintf(temp, "%s%d", f,i);
		//if (stat(temp, &st) == -1) {
			printf("\n%s directory created\n", temp);
			mkdir(temp, 0777);
			sprintf(command,"cp -r %s %s ", source_code, temp); //copying the source files to factory
            system(command);
            sprintf(command,"cp -r %s %s ", data, temp); //copying the data files to factory
            system(command);
            printf("\n fast-lr fit executed \n");
            sprintf(command,"find %s/fast-lr -execdir ./fast-lr fit X_train.csv y_train.csv --verbose --with-intercept ';' ", temp); //copying the data files to factory
            system(command);
            printf("\n fast-lr predict executed \n");
            sprintf(command,"find %s/fast-lr -execdir ./fast-lr predict X_train.csv y_train.csv --verbose --with-intercept ';' ", temp); //copying the data files to factory
            system(command);
		//}
		//else{
	    //    printf("\n%s directory already exists\n", temp);
		//}
	}	
	return 0;
}
