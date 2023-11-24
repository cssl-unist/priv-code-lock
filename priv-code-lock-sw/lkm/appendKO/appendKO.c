#include <stdio.h>

int main(int argc, char *argv[]) {

	FILE *elf_f;
	FILE *hash_f;
	char buffer[256];
	
	hash_f = fopen("hash.txt", "r");
	if (hash_f == NULL) { 
			perror("hash.txt didn't exist\n");
	}

	elf_f = fopen(argv[1], "a"); // append
	
	if(elf_f == NULL) {
		perror("Error opening file\n");
	}
	else {
		while(fgets(buffer, sizeof(buffer), hash_f)) {
			printf("processing ----\n");
			fprintf(elf_f, "%s", buffer);
			}
		}

	fclose(hash_f);
	fclose(elf_f);	

}

