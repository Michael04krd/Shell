#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdbool.h>

void save_history(char*command) { // функция сохранения истории введенных команд
  FILE *file = fopen("history-txt", "a");
  if(file == NULL){ // проверяем файл на пустоту
    perror("Failed to open, file is empty");
    return;
  }
  fprintf(file, "%s\n", command);  // записываем в файл последнюю команду
  fclose(file);
}


bool read_input(char *input, size_t size) {  // функция чтения входной строки
  printf("Enter your string: ");
  if(fgets(input, size, stdin) == NULL) { // проверка на ошибку чтения. Если fgets вернул NULL, то цикл прерывается, выдавая сообщение Exit
    printf("Exit\n");
    return false;
  }
  input[strcspn(input, "\n")] = '\0'; // изменения символа новой строки \n на нулевой символ \0
  return true;
}


bool isEmpty(char *input) { // проверка введенной строки на пустоту, а также на пробелы
  for(size_t index = 0; index < strlen(input); index++) {
    if(!isspace(input[index]))
      return true;
  }
  return false;
}


bool process_exit(char *input) {
  if((strncmp(input, "\\q", 2) == 0) || (strncmp(input, "exit", 4) == 0)) { // Если ввели exit or \q, то выходим
    return true;
  }
  return false;
}


void process_echo(char *input) { // функция-обработка ключевой команды echo, для вывода введенной пользователем строки
  printf("%s\n", input+5); // вывод на экран строки отступая 5 символов от начала (echo )
}


void enviroment_var(char *input) {  // функция вывода переменной окружения 
  char *var_name = input + 3;  // пропустили в строке "\e "
  var_name[strcmp(var_name, "\n")] = 0;  // Убрали символ переноса
  if(var_name ) {
    printf("Incorrect name of the environment variable\n");
  }
  char *var_value = getenv(var_name); // создаем указатель, указывающий на значение переменной окружения

  if(var_value == NULL) {  //Если переменная окружения не найдена
    printf("Environment variable %s\n is not found\n", var_name);
  } else {  // Если нашли, то вывели
    printf("Environment variable: %s\n = %s\n\n", var_name, var_value);
  }
}

void executeCommand(const char* command) {
  pid_t pid = fork();
  if(pid == 0) {
    execlp(command, command, (char*)NULL);
    perror("execlp failed");
    exit(1);
  } else if(pid < 0) {
    perror("fork failed");
  } else {
    wait(NULL);
  }
}

void handle_highup() {
  printf("Configuration reloaded\n\n");
  printf("Enter your string: ");
  fflush(stdout);
}

int main() { 
  signal(1,handle_highup);
  char input[200];
  
  while(1) {
    if(!read_input(input, sizeof(input))) {
      break;
    }

    save_history(input);

    if(process_exit(input)) {
      printf("You successfully exited\n");
      break;
    }

    if(strncmp(input, "echo ", 5) == 0) { // Функция сравнивает 2 строки и возвращает 0, если равны
      process_echo(input);
      continue;
    }


    if(strncmp(input, "\\e ", 3) == 0) {
      enviroment_var(input);
      continue;
    }

    if(!isEmpty(input)) {
      printf("String you entered is empty\n");
      continue;
    }

    if(strncmp(input, "run ", 4) == 0) {
      char* run_command = input + 4;
      run_command[strcspn(run_command, "\n")] = 0;
      executeCommand(run_command);
      printf("\n");
      continue;
    }

    printf("String you entered %s\n", input);
  }
  return 0;
}

