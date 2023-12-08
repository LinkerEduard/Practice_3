#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 1000
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define PORT 6379
#define BACKLOG 10
#define BUFFER_SIZE 104857600
HANDLE mutex;
int need_unic_key = 1;

typedef struct {
    char** elements;
    int top;
} Stack;

Stack* InitStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->elements = (char**)malloc(MAX_SIZE * sizeof(char*));
    stack->top = -1;
    return stack;
}

void SPUSH(Stack* stack, char* element) {
    if (stack->top >= MAX_SIZE - 1) {
        printf("Stack full\n");
        return;
    }
    stack->top++;
    stack->elements[stack->top] = _strdup(element);
}

char* SPOP(Stack* stack) {
    if (stack->top < 0) {
        return NULL;
    }
    char* element = stack->elements[stack->top];
    stack->top--;
    return element;
}

Stack* ExeStack(Stack* other, const char* filename, const char* basename, const char* item, const char* query) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }
    Stack* stack = InitStack();
    char line[MAX_SIZE][MAX_SIZE] = { 0 };
    char c; char tempbase[MAX_SIZE] = { 0 };
    int sch_st = 0; int nal_st = 0; int bel = 1;
    for (int i = 0; i + sch_st < MAX_SIZE - 3; i++) {
        fscanf(file, "%s", line[i]);
        c = line[i][0];
        if (c == '#') {
            bel = 1;
            if ((basename != NULL) && (strcmp(tempbase, basename) == 0) && (strcmp(query, "SPUSH") == 0)) {
                SPUSH(stack, item);
                nal_st += 1;
                sch_st += 1;
            }
            for (int v = 0; v < MAX_SIZE; v++) {
                tempbase[v] = 0;
            }
            for (int j = 1; line[i][j] != '\0'; j++) {
                tempbase[j - 1] = line[i][j];
            }
        }
        if (c == '%' || c == '$' || c == '^') {
            bel = 0;
        }
        if (c == '\n') {
            continue;
        }
        if (bel == 1) {
            SPUSH(stack, line[i]);
        }
        if (bel == 0) {
            SPUSH(other, line[i]);
        }
        if (feof(file)) {
            if ((basename != NULL) && (strcmp(tempbase, basename) == 0) && (strcmp(query, "SPUSH") == 0)) {
                SPUSH(stack, item);
                nal_st += 1;
            }
            if ((nal_st == 0) && (strcmp(query, "SPUSH") == 0)) {
                line[i][0] = '#';
                for (int j = 1; (basename[j - 1] != 0) && (j < MAX_SIZE); j++) {
                    line[i][j] = basename[j - 1];
                }
                SPUSH(stack, line[i]);
                SPUSH(stack, item);
            }
            break;
        }
    }
    fclose(file);
    return stack;
}

void SaveStack(Stack* other, Stack* stack, const char* filename, const char* basename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    for (int i = 0; i <= stack->top; i++) {
        if (i == 0) {
            fprintf(file, "%s", stack->elements[i]);
        }
        else {
            fprintf(file, "\n%s", stack->elements[i]);
        }
    }
    for (int i = 0; i <= other->top; i++) {
        fprintf(file, "\n%s", other->elements[i]);
    }
    fclose(file);
}

typedef struct {
    char** elements;
    int front;
    int rear;
} Queue;

Queue* InitQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->elements = (char**)malloc(MAX_SIZE * sizeof(char*));
    queue->front = -1;
    queue->rear = -1;
    return queue;
}

void QPUSH(Queue* queue, char* element) {
    if ((queue->rear + 1) % MAX_SIZE == queue->front) {
        printf("Queue full\n");
        return;
    }
    if (queue->front == -1) {
        queue->front = 0;
        queue->rear = 0;
    }
    else {
        queue->rear = (queue->rear + 1) % MAX_SIZE;
    }
    queue->elements[queue->rear] = _strdup(element);
}

char* QPOP(Queue* queue, int output) {
    if (queue->front == -1) {
        printf("Queue is empty\n");
        return;
    }
    char* element = queue->elements[queue->front];
    if (queue->front == queue->rear) {
        queue->front = -1;
        queue->rear = -1;
    }
    else {
        queue->front = (queue->front + 1) % MAX_SIZE;
    }
    if (output == 1) {
        return element;
    }
    else { return NULL; }
}

Queue* ExeQueue(Stack* other, const char* filename, const char* basename, const char* item, const char* query) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }
    Queue* queue = InitQueue();
    char line[MAX_SIZE][MAX_SIZE] = { 0 };
    char c;
    char tempbase[MAX_SIZE] = { 0 };
    int sch_st = 0; int bel = 2;
    int nal_st = 0;
    for (int i = 0; i + sch_st < MAX_SIZE - 3; i++) {
        fscanf(file, "%s", line[i]);
        c = line[i][0];
        if (c == '%') {
            bel = 1;
            if ((basename != NULL) && (strcmp(tempbase, basename) == 0) && (strcmp(query, "QPUSH") == 0)) {
                QPUSH(queue, item);
                nal_st += 1;
                sch_st += 1;
            }
            for (int v = 0; v < MAX_SIZE; v++) {
                tempbase[v] = 0;
            }
            for (int j = 1; line[i][j] != '\0'; j++) {
                tempbase[j - 1] = line[i][j];
            }
        }
        if (c == '#' || c == '$' || c == '^') {
            bel = 0;
        }
        if (c == '\n') {
            continue;
        }
        if (bel == 1) {
            QPUSH(queue, line[i]);
        }
        if (bel == 0) {
            SPUSH(other, line[i]);
        }
        if (feof(file)) {
            if ((basename != NULL) && (strcmp(tempbase, basename) == 0) && (strcmp(query, "QPUSH") == 0)) {
                QPUSH(queue, item);
                nal_st += 1;
            }
            if ((nal_st == 0) && (strcmp(query, "QPUSH") == 0)) {
                line[i][0] = '%';
                for (int j = 1; (basename[j - 1] != 0) && (j < MAX_SIZE); j++) {
                    line[i][j] = basename[j - 1];
                }
                QPUSH(queue, line[i]);
                QPUSH(queue, item);
            }
            break;
        }
    }
    fclose(file);
    return queue;
}

void SaveQueue(Stack* other, Queue* queue, const char* filename, const char* basename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    int n = 0;
    while (queue->front != -1) {
        char* element = queue->elements[queue->front];
        fprintf(file, "%s\n", element);
        n = 1;
        QPOP(queue, 0);
    }
    for (int i = 0; i <= other->top; i++) {
        if (n == 1 && i == 0) {
            fprintf(file, "%s", other->elements[i]);
        }
        else {
            fprintf(file, "\n%s", other->elements[i]);
        }
    }
    fclose(file);
}

typedef struct Node {
    char* element;
    char* set_name;
    int hash;
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct Set {
    Node* head;
    int size;
    Node* hashTable[MAX_SIZE];
    int tableSize;
    int* emptySlots;
} Set;


Set* InitSet() {
    Set* set = (Set*)malloc(sizeof(Set));
    set->head = NULL;
    set->size = 0;
    set->tableSize = MAX_SIZE;
    set->emptySlots = (int*)malloc(MAX_SIZE * sizeof(int));
    for (int i = 0; i < MAX_SIZE; i++) {
        set->hashTable[i] = NULL;
        set->emptySlots[i] = 1;
    }
    return set;
}

int HashSet(char* element) {
    int hash = 0;
    for (int i = 0; element[i] != '\0'; i++) {
        hash += element[i];
    }
    return hash % MAX_SIZE;
}

void SADD(Set* set, char* element, char* basename, int need_key) {
    if (strcmp(element, "") == 0) { return; }
    int unic_key = 1;
    int hash = HashSet(element) % set->tableSize;
    int temp_hash = hash;
    while (set->emptySlots[hash] == 0) {
        if ((HashSet(set->hashTable[hash]->element) == temp_hash) && (strcmp(set->hashTable[hash]->set_name, basename) == 0)) {
            unic_key = 0;
        }
        hash = (hash + 1) % set->tableSize;
    }
    if (unic_key == 1) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->element = _strdup(element);
        newNode->set_name = _strdup(basename);
        newNode->hash = hash;
        newNode->next = set->head;
        if (set->head != NULL) {
            set->head->prev = newNode;
        }
        set->head = newNode;
        set->hashTable[hash] = newNode;
        set->emptySlots[hash] = 0;
        set->size++;
    }
    if (need_key == 1) {
        need_unic_key = unic_key;
    }
}

void SREM(Set* set, char* element, char* basename) {
    int hash = HashSet(element) % set->tableSize;
    while (set->hashTable[hash] != NULL) {
        if (strcmp(set->hashTable[hash]->element, element) == 0 && strcmp(set->hashTable[hash]->set_name, basename) == 0) {
            Node* nodeToRemove = set->hashTable[hash];
            if (nodeToRemove == set->head) {
                set->head = nodeToRemove->next;
            }
            else {
                if (nodeToRemove->prev != NULL) {
                    nodeToRemove->prev->next = nodeToRemove->next;
                }
            }
            if (nodeToRemove->next != NULL) {
                nodeToRemove->next->prev = nodeToRemove->prev;
            }
            free(nodeToRemove->element);
            free(nodeToRemove);
            set->hashTable[hash] = NULL;
            set->emptySlots[hash] = 1;
            set->size--;
        }
        else {
            break;
        }
        hash = (hash + 1) % set->tableSize;
    }
}

int SISMEMBER(Set* set, char* element, char* basename) {
    int hash = HashSet(element) % set->tableSize;
    for (int i = 0; i < MAX_SIZE; i++) {
        Node* current = set->hashTable[hash];
        if ((current != NULL) && ((current->element != 0) && (current->set_name != 0)) && (strcmp(current->element, element) == 0 && strcmp(current->set_name, basename) == 0)) {
            return 1;
        }
        if (current == NULL) {
            return 0;
        }
        hash = (hash + 1) % set->tableSize;
    }
    return 0;
}

Set* ExeSet(Stack* other, char* filename, char* basename, char* item, char* query) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }
    Set* set = InitSet();
    char line[MAX_SIZE][MAX_SIZE] = { 0 };
    int sch_set = 0; int nal_set = 0; int sch_n = 0; int sch = 0; int bel = 2;
    char c; char tempbase[MAX_SIZE] = { 0 };
    for (int i = 0; i + sch_set < MAX_SIZE - 3; i++) {
        fscanf(file, "%s", line[i]);
        c = line[i][0];
        if (c == '$') {
            bel = 1;
            if ((strcmp(tempbase, basename) == 0) && (strcmp(query, "SADD") == 0)) {
                SADD(set, item, basename, 1);
                nal_set += 1;
                sch_set += 1;
            }
            for (int v = 0; v < MAX_SIZE; v++) {
                tempbase[v] = 0;
            }
            for (int j = 1; line[i][j] != '\0'; j++) {
                tempbase[j - 1] = line[i][j];
            }
        }
        if (c == '#' || c == '%' || c == '^') {
            bel = 0;
        }
        if (c == '\n') {
            sch_n += 1;
            continue;
        }
        if (bel == 1) {
            if ((c != '$') && (c != '\n')) {
                SADD(set, line[i], tempbase, 0);
            }
        }
        if (bel == 0) {
            SPUSH(other, line[i]);
        }
        if (feof(file)) {
            if ((strcmp(tempbase, basename) == 0) && (strcmp(query, "SADD") == 0)) {
                SADD(set, item, basename, 1);
                nal_set += 1;
            }
            if ((nal_set == 0) && (strcmp(query, "SADD") == 0)) {
                SADD(set, item, basename, 1);
            }
            break;
        }
    }
    fclose(file);
    return set;
}

void SaveSet(Stack* other, Set* set, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("ќшибка открыти€ файла\n");
        return;
    }
    int n = 0;
    while (1) {
        Node* current = set->head;
        if (current == NULL || current->element == NULL || current->set_name == NULL) { goto jump; }
        Node* pred = current;
        char* basename = current->set_name;
        fprintf(file, "$%s\n", basename);
        while (current != NULL) {
            if (strcmp(basename, current->set_name) == 0) {
                fprintf(file, "%s\n", current->element);
                n = 1;
                Node* vrem = current->next;
                if (set->head == current) { set->head = vrem; }
                current->element = NULL;
                current->set_name = NULL;
                pred->next = current->next;
                current = NULL;
                current = vrem;
            }
            else {
                pred = current;
                current = current->next;
            }
        }
    }
jump:
    for (int i = 0; i <= other->top; i++) {
        if (n == 1 && i == 0) {
            fprintf(file, "%s", other->elements[i]);
        }
        else {
            fprintf(file, "\n%s", other->elements[i]);
        }
    }
    fclose(file);
}

typedef struct entry {
    char* key;
    char* value;
    struct entry* next;
    struct entry* prev;
    struct entry* coll_next;
    struct entry* coll_prev;
    char* tableName;
} Entry;

typedef struct {
    Entry* elements;
    Entry* hashTable[MAX_SIZE];
    int size;
} HashTable;

int HashforTable(char* key, int size) {
    int sum = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        sum += key[i];
    }
    return sum % size;
}

HashTable* InitTable(int size) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->elements = (Entry*)malloc(size * sizeof(Entry));
    for (int i = 0; i < MAX_SIZE; i++) {
        table->hashTable[i] = NULL;
    }
    table->elements = NULL;
    table->size = size;
    return table;
}

void HSET(HashTable* table, char* key, char* value, char* tableName, int need_key) {
    int index = HashforTable(key, table->size);
    Entry* entry = (Entry*)malloc(sizeof(Entry));
    entry->key = (char*)malloc(strlen(key) + 1);
    strcpy(entry->key, key);
    entry->value = (char*)malloc(strlen(value) + 1);
    strcpy(entry->value, value);
    entry->tableName = (char*)malloc(strlen(tableName) + 1);
    strcpy(entry->tableName, tableName);
    entry->next = NULL;
    entry->prev = NULL;
    entry->coll_next = NULL;
    entry->coll_prev = NULL;
    Entry* current = table->elements;

    int unic_key = 0;
    Entry* temp = table->hashTable[index];
    if (table->hashTable[index] == NULL) {
        unic_key = 1;
    }
    while ((temp != NULL) && (strcmp(temp->key, entry->key) != 0)) {
        temp = temp->coll_prev;
        if (temp == NULL) {
            unic_key = 1;
        }
    }


    if (unic_key == 1) {
        if (current == NULL) {
            table->elements = entry;
        }
        else {
            entry->next = current;
            current->prev = entry;
            table->elements = entry;
        }
        if (table->hashTable[index] == NULL) {
            table->hashTable[index] = entry;
        }
        else {
            entry->coll_next = table->hashTable[index];
            table->hashTable[index]->coll_prev = entry;
            table->hashTable[index] = entry;
        }
    }
    if (need_key == 1) {
        need_unic_key = unic_key;
    }
}



int HFIND(HashTable* table, char* basename, char* item, char* key) {
    int index = HashforTable(key, table->size);
    Entry* current = table->hashTable[index];
    while (1) {
        if (current == NULL) {
            return 0;
        }
        if ((current != NULL) && ((current->key != NULL) && (current->tableName != NULL) && (current->value != NULL) && (strcmp(current->key, key) == 0) && (strcmp(current->tableName, basename) == 0) && (strcmp(current->value, item) == 0))) {
            return 1;
        }
        else {
            current = current->coll_next;
        }
    }
}

char* HGET(HashTable* table, char* basename, char* key) {
    int index = HashforTable(key, table->size);
    Entry* current = table->hashTable[index];
    while (1) {
        if (current == NULL) {
            return NULL;
        }
        if ((current != NULL) && ((current->key != NULL) && (current->tableName != NULL) && (current->value != NULL) && (strcmp(current->key, key) == 0) && (strcmp(current->tableName, basename) == 0))) {
            return current->value;
        }
        else {
            current = current->coll_next;
        }
    }
}

int HDEL(HashTable* table, char* basename, char* item, char* key) {
    if (HFIND(table, basename, item, key) == 1) {
        int index = HashforTable(key, table->size);
        Entry* current = table->hashTable[index];
        Entry* vrem = NULL;
        while (current != NULL) {
            if ((current->key != NULL) && (current->tableName != NULL) && (current->value != NULL) && (strcmp(current->key, key) == 0) && (strcmp(current->tableName, basename) == 0) && (strcmp(current->value, item) == 0)) {
                if (table->elements == current) {
                    table->elements = current->next;
                }
                else {
                    current->prev->next = current->next;
                    if (current->next != NULL) { current->next->prev = current->prev; }
                }
                if (current->coll_prev == NULL) {
                    table->hashTable[index] = current->coll_next;
                    vrem = current->coll_next;
                    current->key = NULL;
                    current->value = NULL;
                    current->tableName = NULL;
                    free(current);
                }
                else {
                    current->coll_prev->coll_next = current->coll_next;
                    vrem = current->coll_next;
                    current->key = NULL;
                    current->value = NULL;
                    current->tableName = NULL;
                    free(current);
                }
                current = vrem;
            }
            else {
                current->coll_prev = current;
                current = current->coll_next;
            }
        }
        return 1;
    }
    else {
        return 0;
    }
}

HashTable* ExeTable(Stack* other, char* filename, char* basename, char* item, char* key, char* query) {
    FILE* file = fopen(filename, "r");
    HashTable* table = InitTable(MAX_SIZE);
    char line[MAX_SIZE][MAX_SIZE] = { 0 };
    char c; char tempbase[MAX_SIZE] = { 0 }; int bel = 2;
    int sch_hash = 0; int nal_hash = 0; char temp_key[MAX_SIZE] = { 0 }; char temp_line[MAX_SIZE] = { 0 }; int flag = 0;
    for (int i = 0; i + sch_hash < MAX_SIZE - 3; i++) {
        fscanf(file, "%s", line[i]);
        c = line[i][0];
        if (c == '^') {
            bel = 1;
            if ((strcmp(tempbase, basename) == 0) && (strcmp(query, "HSET") == 0)) {
                HSET(table, key, item, basename, 1);
                flag = 1;
                nal_hash += 1;
                sch_hash += 1;
            }
            for (int a = 1; a < MAX_SIZE; a++) {
                tempbase[a] = 0;
            }
            for (int j = 1; line[i][j] != '\0'; j++) {
                tempbase[j - 1] = line[i][j];
            }
            for (int v = 0; v < MAX_SIZE; v++) {
                temp_key[v] = 0;
            }
            for (int f = 0; f < MAX_SIZE; f++) {
                temp_line[f] = 0;
            }
        }
        if (c == '#' || c == '$' || c == '%') {
            bel = 0;
        }
        if (c == '\n') {
            continue;
        }
        if (bel == 1) {
            if (c == '~') {
                for (int v = 0; v < MAX_SIZE; v++) {
                    temp_key[v] = 0;
                }
                int j = 1;
                for (; line[i][j] != '~' && j < MAX_SIZE; j++) {
                    temp_key[j - 1] = line[i][j];
                }
                for (int f = 0; f < MAX_SIZE; f++) {
                    temp_line[f] = 0;
                }
                for (int g = j + 1; g < MAX_SIZE; g++) {
                    temp_line[g - j - 1] = line[i][g];
                }
            }
            if (c == '~') {
                HSET(table, temp_key, temp_line, tempbase, 0);
            }
        }
        if (bel == 0) {
            SPUSH(other, line[i]);
        }
        if (feof(file)) {
            if ((strcmp(tempbase, basename) == 0) && (strcmp(query, "HSET") == 0) && (flag == 0)) {
                HSET(table, key, item, basename, 1);
                nal_hash += 1;
            }
            if ((nal_hash == 0) && (strcmp(query, "HSET") == 0)) {
                HSET(table, key, item, basename, 1);
            }
            break;
        }
    }
    fclose(file);
    return table;
}

void SaveTable(Stack* other, HashTable* table, char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    int n = 0;
    while (table->elements != NULL) {
        Entry* current = table->elements;
        if (current == NULL || current->value == NULL || current->tableName == NULL || current->key == NULL) { break; }
        Entry* pred = current;
        char* basename = current->tableName;
        fprintf(file, "^%s\n", basename);
        while (current != NULL) {
            if (current->tableName != 0 && strcmp(basename, current->tableName) == 0) {
                fprintf(file, "~%s~%s\n", current->key, current->value);
                n = 1;
                Entry* vrem = current->next;
                if (table->elements == current) { table->elements = vrem; }
                current->value = NULL;
                current->tableName = NULL;
                pred->next = current->next;
                current = NULL;
                current = vrem;
            }
            else {
                pred = current;
                current = current->next;
            }
        }
    }
    for (int i = 0; i <= other->top; i++) {
        if (n == 1 && i == 0) {
            fprintf(file, "%s", other->elements[i]);
        }
        else {
            fprintf(file, "\n%s", other->elements[i]);
        }
    }
    fclose(file);
}
DWORD WINAPI handle_client(LPVOID lpParam) {
    SOCKET ClientSock = (SOCKET)lpParam;
    while (1) { // добавили цикл while, который будет повтор€тьс€, пока сокет клиента не будет закрыт, или пока не произойдет ошибка
        char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
        int bytes_received; // добавили переменную дл€ хранени€ количества байт, полученных от клиента
        do { // добавили цикл do-while, который будет повтор€тьс€, пока не будет получен весь запрос от клиента, или пока не произойдет ошибка
            // получаем часть запроса и сохран€ем ее в буфере
            bytes_received = recv(ClientSock, buffer, BUFFER_SIZE, 0);
            if (bytes_received == SOCKET_ERROR) { // провер€ем, что не произошла ошибка
                printf("recv failed: %d\n", WSAGetLastError());
                break;
            }
            if (bytes_received == 0) { // провер€ем, что клиент не отключилс€
                printf("client disconnected\n");
                break;
            }
        } while (bytes_received == BUFFER_SIZE); // повтор€ем, пока не получим меньше, чем размер буфера, что означает, что запрос закончилс€
        if (bytes_received <= 0) { // если произошла ошибка или отключение, выходим из цикла
            break;
        } // повтор€ем, пока не получим меньше, чем размер буфера, что означает, что запрос закончилс€
        buffer[bytes_received] = '\0'; // добавл€ем нулевой символ в конец буфера, чтобы обозначить конец строки
        // дальше обрабатываем запрос, как вы делали раньше, использу€ буфер как источник данных


        char* argv[500] = {0};
        int argc = 0;
        char delimiters[] = " ";
        char* word = strtok(buffer, delimiters);
        while (word != NULL) {
            argv[argc++] = word;
            word = strtok(NULL, delimiters);
        }
        char* filename = NULL;
        char* key = NULL;
        char* query = NULL;
        char* item = NULL;
        char* basename = NULL;
        int hdel = 0;
        char* exodus = NULL;

        if (((argc == 4) || (argc == 6) || (argc == 7)) && (strcmp(argv[0], "--file") == 0) && (strcmp(argv[2], "--query") == 0)) {
            filename = argv[1];
            if ((argc == 6) && (strcmp(argv[3], "HGET") != 0)) {
                query = argv[3];
                basename = argv[4];
                item = argv[5];
            }
            else if (argc == 4) {
                query = argv[3];
            }
            else if ((argc == 6) && (strcmp(argv[3], "HGET") == 0)) {
                query = argv[3];
                basename = argv[4];
                key = argv[5];
            }
            else if (argc == 7) {
                query = argv[3];
                basename = argv[4];
                item = argv[6];
                key = argv[5];
            }
            else if ((argc != 4) && (argc != 6) && (argc != 7)) {
                exodus = malloc(200);
                sprintf(exodus, "Error in number of arguments in query\n"); return 0;
            }
            int len = strlen(filename);
            char* last_four = NULL;
            if (len > 4) { last_four = &filename[len - 4]; }
            if (strcmp(last_four, ".txt") != 0) {
                exodus = malloc(200);
                sprintf(exodus, "File name error (please use a text file)\n"); return 0;
            }
            FILE* file;
            file = fopen(filename, "r");
            if (file == NULL) {
                file = fopen(filename, "w");
            }
            if ((strcmp(query, "SPUSH") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                Stack* stack = ExeStack(other, filename, basename, item, query);
                fclose(file);
                SaveStack(other, stack, filename, basename);
                exodus = malloc(strlen(item) + strlen(basename) + 100);
                sprintf(exodus, "basename: %s, item: %s", basename, item);
            }
            if ((strcmp(query, "SPOP") == 0) && (argc == 4)) {
                Stack* other = InitStack();
                Stack* stack = ExeStack(other, filename, basename, item, query);
                char* vrem = NULL;
                vrem = SPOP(stack);
                exodus = malloc(strlen(vrem) + 100);
                sprintf(exodus, "item or basename:%s\n", vrem);
                fclose(file);
                SaveStack(other, stack, filename, basename);

            }
            if ((strcmp(query, "QPUSH") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                Queue* queue = ExeQueue(other, filename, basename, item, query);
                fclose(file);
                SaveQueue(other, queue, filename, basename);
                exodus = malloc(strlen(item) + strlen(basename) + 100);
                sprintf(exodus, "basename: %s, item: %s", basename, item);
            }
            if ((strcmp(query, "QPOP") == 0) && (argc == 4)) {
                Stack* other = InitStack();
                Queue* queue = ExeQueue(other, filename, basename, item, query);
                char* vrem = QPOP(queue, 1);
                fclose(file);
                SaveQueue(other, queue, filename, basename);
                exodus = malloc(strlen(vrem) + 100);
                sprintf(exodus, "basename or item: %s", vrem);
            }
            if ((strcmp(query, "SADD") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                Set* set = ExeSet(other, filename, basename, item, query);

                if (need_unic_key == 1) {
                    exodus = malloc(strlen(item) + strlen(basename) + 100);
                    sprintf(exodus, "basename: %s, item: %s", basename, item);
                }
                else {
                    exodus = malloc(200);
                    sprintf(exodus, "This key is already in use, please enter another key\n");
                }
                SaveSet(other, set, filename);
                fclose(file);
            }
            if ((strcmp(query, "SREM") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                Set* set = ExeSet(other, filename, basename, item, query);
                if (SISMEMBER(set, item, basename)) {
                    SREM(set, item, basename);
                    exodus = malloc(strlen(item) + strlen(basename) + 100);
                    sprintf(exodus, "basename:%s, element:%s\n", basename, item);
                }
                else {
                    exodus = malloc(200);
                    sprintf(exodus, "%s\n", "Error, element missing");
                }
                SaveSet(other, set, filename);
                fclose(file);
            }
            if ((strcmp(query, "SISMEMBER") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                Set* set = ExeSet(other, filename, basename, item, query);
                if (SISMEMBER(set, item, basename)) {
                    exodus = malloc(200);
                    sprintf(exodus, "%s\n", "TRUE");
                }
                else {
                    exodus = malloc(200);
                    sprintf(exodus, "%s\n", "FALSE");
                }
                SaveSet(other, set, filename);
                fclose(file);
            }
            if ((strcmp(query, "HSET") == 0) && (argc == 7)) {
                Stack* other = InitStack();
                HashTable* table = ExeTable(other, filename, basename, item, key, query);
                SaveTable(other, table, filename);
                fclose(file);
                if (need_unic_key == 1) {
                    exodus = malloc(strlen(key) + strlen(item) + strlen(basename) + 100);
                    sprintf(exodus, "key: %s, item: %s, basename: %s\n", key, item, basename);
                }
                else {
                    exodus = malloc(200);
                    sprintf(exodus, "This key is already in use, please enter another key\n");
                }

            }
            if ((strcmp(query, "HDEL") == 0) && (argc == 7)) {
                Stack* other = InitStack();
                HashTable* table = ExeTable(other, filename, basename, item, key, query);
                if (HDEL(table, basename, item, key)) {
                    exodus = malloc(strlen(key) + strlen(item) + strlen(basename) + 100);
                    sprintf(exodus, "key: %s, item: %s, basename: %s\n", key, item, basename);
                }
                else {
                    exodus = malloc(200);
                    sprintf(exodus, "%s", "Error, element was not deleted\n");
                }
                SaveTable(other, table, filename);
                fclose(file);
            }
            if ((strcmp(query, "HGET") == 0) && (argc == 6)) {
                Stack* other = InitStack();
                HashTable* table = ExeTable(other, filename, basename, item, key, query);
                char* value = NULL;
                value = HGET(table, basename, key);
                if (value == NULL) {
                    exodus = malloc(200);
                    sprintf(exodus, "False\n");
                }
                else {
                    exodus = malloc(strlen(value) + 10);
                    sprintf(exodus, "%s\n", value);
                }
                SaveTable(other, table, filename);
                fclose(file);
            }
        }
        else {
            exodus = malloc(200);
            sprintf(exodus, "Invalid command syntax\n");
        }
        int bytes_sent = send(ClientSock, exodus, strlen(exodus), 0);
        free(exodus);
        free(buffer);
    }
    closesocket(ClientSock);
}

int main() {
    WSADATA wsaData;
    SOCKET ServerSock;
    SOCKADDR_IN ServerInfo;
    int exodus;

    // »нициализируем библиотеку Winsock
    exodus = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (exodus != 0) {
        printf("WSAStartup failed: %d\n", exodus);
        return 1;
    }

    // —оздаем сокет дл€ сервера
    ServerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // ѕрив€зываем сокет к адресу и порту
    ServerInfo.sin_family = AF_INET;
    ServerInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerInfo.sin_port = htons(PORT);
    exodus = bind(ServerSock, (SOCKADDR*)&ServerInfo, sizeof(ServerInfo));
    if (exodus == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(ServerSock);
        WSACleanup();
        return 1;
    }

    // ѕереводим сокет в режим прослушивани€
    exodus = listen(ServerSock, BACKLOG);
    if (exodus == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(ServerSock);
        WSACleanup();
        return 1;
    }

    // ѕринимаем вход€щие соединени€ от клиентов
    printf("Server is listening on port %d\n", PORT);
    while (1) {
        SOCKET ClientSock;
        SOCKADDR_IN ClientAddr;
        int ClientAddrSize = sizeof(ClientAddr);

        // ѕринимаем соединение от клиента
        ClientSock = accept(ServerSock, (SOCKADDR*)&ClientAddr, &ClientAddrSize);
        if (ClientSock == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(ServerSock);
            WSACleanup();
            return 1;
        }

        // —оздаем новый поток дл€ обработки запроса клиента
        printf("Accepted connection from %s:%d\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(NULL, 0, handle_client, (LPVOID)ClientSock, 0, &ThreadID);
        if (ThreadHandle == NULL) {
            printf("CreateThread failed: %d\n", GetLastError());
            closesocket(ClientSock);
            closesocket(ServerSock);
            WSACleanup();
            return 1;
        }
        CloseHandle(ThreadHandle);
    }

    // «акрываем сокет сервера и очищаем библиотеку Winsock
    closesocket(ServerSock);
    WSACleanup();
    return 0;
}