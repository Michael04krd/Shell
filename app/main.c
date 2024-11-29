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

void clearScreen() {
    printf("\033[H\033[J");  // clear ANSI-code
    fflush(stdout);          // Сбрасываем буфер, чтобы вывод сразу применился
}

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
  if(!isEmpty(var_name)) {
    printf("The environment variable can't be an empty string\n");  // проверили переменную окружения на пустоту
    return;
  }
  char *var_value = getenv(var_name); // создаем указатель, указывающий на значение переменной окружения

  if(var_value == NULL) {  //Если переменная окружения не найдена
    printf("Environment variable %s is not found\n", var_name);
  } else {  // Если нашли, то вывели
    printf("Environment variable: %s\n = %s\n\n", var_name, var_value);
  }
}

void executeCommand(const char* command) {  // выполнение бинарника
  pid_t pid = fork();  // с помощью системного вызова создаем новый процесс
  if(pid == 0) {  // если процесс дочерний
    execlp(command, command, (char*)NULL); // ищем нашу команду в каталогах PATH
    perror("execlp failed");  // если не нашли команду
    exit(1);
  } else if(pid < 0) {  // произошла ошибка при создании нового процесса 
    perror("fork failed");
  } else {
    wait(NULL);  // родительский процесс ждет завершения дочернего
  }
}

void handle_sighup() {
  printf("Configuration reloaded\n\n");
  printf("Enter your string: ");
  fflush(stdout);
}

bool sys_sections(char* name_device) {
  char device_path[256]; // тут будет храниться путь к утройству
  snprintf(device_path, sizeof(device_path), "/dev/%s", name_device); // объединяем строку с /dev/ и результат записываем в devixe_path
  int device = open(device_path, O_RDONLY); // открывает устройство по пути в режиме read only
  
  if(device == -1) {  // если не смогли открыть
    perror("Произошла ошибка при открытии устройства, возможно, путь указан неправильно");
    return true;
  }

  unsigned char sector[512];
  ssize_t read_bytes = read(device, sector, 512); // функция читает 512 байт данных из файла, на который указывает device, сохраняет их в sector

  if(read_bytes == -1) { // если во время чтения произошла ошибка
    perror("Произошла ошибка при чтении сектора");
    return true;
  }
  else if(read_bytes < 512) { // проверка, если было прочитано меньше 512 байт
    perror("Не удалось прочитать сектор полностью");
    close(device);
    return true;
  } 
  
  unsigned short signature;  // переменная для хранения сигнатуры
  signature = (sector[510] << 8) | sector[511]; // извлекается сигнатура из последних 2 байт прочитанного сектора. 0x55AA обычно находится в последних 2-х байтах первого сектора. 
  // формируется полное значение сигнатуры, объединяя 511 и 510 сдвинутый байты

  if(signature == 0x55AA) { // проверка сигнатуры. Запуск через sudo ./main, посмотреть диски df -h, выбор диска \l nvme0n1p1
    printf("Устройство %s является загрузочным\n", name_device);
  } else {
    printf("Устройство %s не является загрузочным\n", name_device);
  }
  close(device);
  return false;
}

int main() { 
  signal(1,handle_sighup);
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

    if (strcmp(input, "clear") == 0) {
      clearScreen();
      continue;  
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

    if(strncmp(input, "\\l ", 3) == 0) {
      char* name_device = input + 3;
      name_device[strcspn(name_device, "\n")] == 0;
      sys_sections(name_device);
      continue;
    }

    printf("String you entered %s\n", input);
  }
  return 0;
}

