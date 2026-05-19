#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"

void process(const char *in_path, const char *out_path) {
    FILE *fin  = fopen(in_path,  "r");
    FILE *fout = fopen(out_path, "w");
    if (!fin)  { fprintf(stderr, "Не удалось открыть %s\n", in_path);  return; }
    if (!fout) { fprintf(stderr, "Не удалось создать %s\n", out_path); fclose(fin); return; }
 
    BTree *tree = btree_create();
    char line[256];
    while (fgets(line, sizeof(line), fin)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;
 
        fprintf(fout, "> %s\n", line);
 
        int op, key;
        char value[MAX_VALUE_LEN];
        int parsed = sscanf(line, "%d %d %s", &op, &key, value);
 
        if (parsed < 1) {
            fprintf(fout, "  ошибка: пустая строка\n\n");
            continue;
        }
 
        switch (op) {
            case 1:
                if (parsed < 3) { 
                    fprintf(fout, "  ошибка: нужен ключ и значение (1 <ключ> <значение>)\n\n"); 
                    break; 
                }
                btree_insert(tree, key, value);
                fprintf(fout, "  вставлен ключ %d со значением '%s'\n\n", key, value);
                break;
 
            case 2:
                if (parsed < 2) { 
                    fprintf(fout, "  ошибка: нужен ключ (2 <ключ>)\n\n"); 
                    break; 
                }
                if (btree_search(tree, key)) {
                    btree_delete(tree, key);
                    fprintf(fout, "  удалён ключ %d\n\n", key);
                } else {
                    fprintf(fout, "  ключ %d не найден\n\n", key);
                }
                break;
 
            case 3:
                fprintf(fout, "  дерево:\n");
                btree_print(tree, fout);
                fprintf(fout, "\n");
                break;
 
            case 4:
                if (parsed < 2) { 
                    fprintf(fout, "  ошибка: нужен ключ (4 <ключ>)\n\n"); 
                    break; 
                }
                char *found = btree_search(tree, key);
                if (found)
                    fprintf(fout, "  ключ %d найден, значение = '%s'\n\n", key, found);
                else
                    fprintf(fout, "  ключ %d не найден\n\n", key);
                break;
 
            default:
                fprintf(fout, "  ошибка: неизвестная операция %d\n\n", op);
        }
    }
 
    btree_free(tree);
    fclose(fin);
    fclose(fout);
    printf("готово → %s\n", out_path);
}

void interactive_mode(void) {
    BTree *tree = btree_create();
    char line[256];
    int op, key;
    char value[MAX_VALUE_LEN];
    
    printf("  B-ДЕРЕВО (t=2) - ИНТЕРАКТИВНЫЙ РЕЖИМ\n");
    printf("  1 <ключ> <знач>  - Добавить узел\n");
    printf("  2 <ключ>         - Удалить узел\n");
    printf("  3                - Печать дерева\n");
    printf("  4 <ключ>         - Поиск значения\n");
    printf("  0                - Выход\n");
    
    while (1) {
        printf("\n> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        
        if (strcmp(line, "0") == 0) {
            printf("До свидания!\n");
            break;
        }
        
        int parsed = sscanf(line, "%d %d %s", &op, &key, value);
        
        if (parsed < 1) {
            printf("  Ошибка: пустая команда\n");
            continue;
        }
        
        switch (op) {
            case 1:
                if (parsed < 3) {
                    printf("  Ошибка: нужен ключ и значение (1 <ключ> <значение>)\n");
                } else {
                    btree_insert(tree, key, value);
                    printf("  Вставлен ключ %d со значением '%s'\n", key, value);
                }
                break;
                
            case 2:
                if (parsed < 2) {
                    printf("  Ошибка: нужен ключ (2 <ключ>)\n");
                } else {
                    char *found = btree_search(tree, key);
                    if (found) {
                        btree_delete(tree, key);
                        printf(" Удалён ключ %d\n", key);
                    } else {
                        printf(" Ключ %d не найден\n", key);
                    }
                }
                break;
                
            case 3:
                printf("  Дерево:\n");
                if (tree->root && tree->root->n > 0) {
                    btree_print(tree, stdout);
                } else {
                    printf("    (пусто)\n");
                }
                break;
                
            case 4:
                if (parsed < 2) {
                    printf("  Ошибка: нужен ключ (4 <ключ>)\n");
                } else {
                    char *found = btree_search(tree, key);
                    if (found) {
                        printf(" Ключ %d найден, значение = '%s'\n", key, found);
                    } else {
                        printf(" Ключ %d не найден\n", key);
                    }
                }
                break;
                
            default:
                printf("  Ошибка: неизвестная операция '%d'\n", op);
        }
    }
    
    btree_free(tree);
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc >= 2 && strcmp(argv[1], "-i") == 0)) {
        interactive_mode();
    } else {
        const char *in  = argv[1];
        const char *out = (argc >= 3) ? argv[2] : "result.txt";
        process(in, out);
    }
    return 0;
}