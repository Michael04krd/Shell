#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void save_history(char*command) {
  FILE *file = fopen("history-txt", "a");
  if(file == NULL){
    perror("Failed to open, file is empty");
    return;
  }
  fprintf(file, "%s\n", command);
  fclose(file);
}

int main() {
  char input[200];
  
  while(1) {
    save_history(input);
    printf("Enter your string: ");
    if(fgets(input, sizeof(input), stdin) == NULL) {
      printf("Exit\n");
      break;
    }

    input[strcspn(input, "\n")] = 0;

    if(strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0) {
      printf("exit\n");
      break;
    }

    if (strncmp(input, "echo ", 5) == 0) {
      printf("%s\n", input+5);
      continue;
    }

    if(strcmp(input, "") == 0){
      printf("enter correct command dibil\n");
      continue;
    }

    printf("String you entered: %s\n", input);

    if(strncmp(input, "\\e ", 3) == 0) {
      char* varName = input+3;
      varName[strcmp(varName, "\n")] = 0;
      char* varValue = getenv(varName);
      if(varValue == NULL) {
        printf("Var not found: %s\n", varName);
      }
      else {
        printf("%s\n", varValue);
      }
    }
  }


  return 0;
}

