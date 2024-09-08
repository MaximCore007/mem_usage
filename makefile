# Имя компилируемого исполняемого файла
TARGET = mem_usage 

BIN = bin

# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -g

# Библиотеки компиляции
LIBS := -lprocps
#INCS := -I/usr/include/proc

# Исходные файлы
SRCS = mem_usage.c

# Объектные файлы
OBJS = $(SRCS:.c=.o)

# Правило по умолчанию (сборка проекта)
all: $(TARGET)
	mv -v $(TARGET) ./$(BIN)/ 

# Правило для сборки исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) ${LIBS}

# Правило для компиляции исходных файлов в объектные файлы
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для очистки сборки
clean:
	rm -f $(OBJS) $(TARGET)

# Правило для сборки проекта, если Makefile изменился
.PHONY: all clean
