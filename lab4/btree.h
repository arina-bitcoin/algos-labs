#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T 2                                      // минимальная степень
#define MAX_KEYS (2 * T - 1)                     // = 3
#define MAX_CHILDREN (2 * T)                     // = 4
#define MIN_KEYS_NON_ROOT (T - 1)                // = 1
#define MAX_VALUE_LEN 256

typedef struct Node {
    int leaf;                                    // 1 - лист, 0 - внутренний
    int n;                                       // текущее количество ключей
    int keys[MAX_KEYS];                          // массив ключей
    char values[MAX_KEYS][MAX_VALUE_LEN];        // массив значений
    struct Node* children[MAX_CHILDREN];         // от 2 до 4 детей (если не лист)
} Node;

typedef struct BTree {
    int t;                                       // минимальная степень (t=2)
    Node* root;                                  // корень (от 1 до 3 ключей)
} BTree;

// Создание дерева
BTree* btree_create(void);

// Создание узла
Node* create_node(int leaf);

// Вставка (с значением)
void btree_insert(BTree *tree, int key, const char *value);
void btree_insert_nonfull(Node *node, int key, const char *value);
void btree_split_child(Node *parent, int index, Node *child);

// Удаление
void btree_delete(BTree *tree, int key);
void btree_delete_internal(Node *node, int key);
int btree_get_predecessor(Node *node, char *value);
int btree_get_successor(Node *node, char *value);
void btree_merge(Node *parent, int index);
void btree_borrow_from_left(Node *parent, int index);
void btree_borrow_from_right(Node *parent, int index);
void btree_fix_child(Node *parent, int index);

// Поиск (возвращает значение или NULL)
char* btree_search(BTree *tree, int key);
char* btree_search_node(Node *node, int key);

// Печать
void btree_print(BTree *tree, FILE *output);
void btree_print_node(Node *node, int level, FILE *output);

// Очистка
void btree_free(BTree *tree);
void btree_free_node(Node *node);

#endif