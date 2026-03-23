#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 13
#define MAX_KEY 64
#define MAX_VAL 256

typedef struct {
    char keys[MAX_N][MAX_KEY];
    char vals[MAX_N][MAX_VAL];
    int n;
} Table;

static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

static int read_line(char *buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

static int read_line_from(FILE *in, char *buf, size_t cap) {
    if (!fgets(buf, (int)cap, in)) return 0;
    trim_newline(buf);
    return 1;
}

static void print_table(const Table *t, const char *title) {
    int i;
    printf("\n%s\n", title);
    printf("%-3s  %-20s  %s\n", "№", "KEY", "DATA");
    for (i = 0; i < t->n; i++) {
        printf("%-3d  %-20s  %s\n", i, t->keys[i], t->vals[i]);
    }
}

static void swap_rows(Table *t, int i, int j) {
    char tmpK[MAX_KEY];
    char tmpV[MAX_VAL];
    strcpy(tmpK, t->keys[i]);
    strcpy(tmpV, t->vals[i]);
    strcpy(t->keys[i], t->keys[j]);
    strcpy(t->vals[i], t->vals[j]);
    strcpy(t->keys[j], tmpK);
    strcpy(t->vals[j], tmpV);
}

static void insertion_sort(Table *t) {
    int i, j;
    for (i = 1; i < t->n; i++) {
        char keyBuf[MAX_KEY];
        char valBuf[MAX_VAL];
        strcpy(keyBuf, t->keys[i]);
        strcpy(valBuf, t->vals[i]);

        j = i - 1;
        while (j >= 0 && strcmp(t->keys[j], keyBuf) > 0) {
            strcpy(t->keys[j + 1], t->keys[j]);
            strcpy(t->vals[j + 1], t->vals[j]);
            j--;
        }
        strcpy(t->keys[j + 1], keyBuf);
        strcpy(t->vals[j + 1], valBuf);
    }
}

static int binary_search_key(const Table *t, const char *key) {
    int l = 0, r = t->n - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        int c = strcmp(t->keys[m], key);
        if (c == 0) return m;
        if (c < 0) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

static void copy_table(Table *dst, const Table *src) {
    int i;
    dst->n = src->n;
    for (i = 0; i < src->n; i++) {
        strcpy(dst->keys[i], src->keys[i]);
        strcpy(dst->vals[i], src->vals[i]);
    }
}

static void reverse_table(Table *t) {
    int i = 0, j = t->n - 1;
    while (i < j) {
        swap_rows(t, i, j);
        i++;
        j--;
    }
}

static int read_table_from(FILE *in, Table *t) {
    char buf[256];
    int n, i;

    if (!read_line_from(in, buf, sizeof(buf))) return 0;
    n = atoi(buf);
    if (n < 1 || n > MAX_N) {
        printf("Ошибка: n должно быть в диапазоне 1..%d\n", MAX_N);
        return 0;
    }
    t->n = n;

    for (i = 0; i < n; i++) {
        if (!read_line_from(in, t->keys[i], sizeof(t->keys[i]))) return 0;
        if (t->keys[i][0] == '\0') {
            printf("Ошибка: KEY не должен быть пустым.\n");
            return 0;
        }
        if (!read_line_from(in, t->vals[i], sizeof(t->vals[i]))) return 0;
    }

    return 1;
}

static void sort_and_search(const Table *base, void (*mutator)(Table *), const char *case_name) {
    Table t;
    char key[MAX_KEY];

    copy_table(&t, base);
    if (mutator) mutator(&t);

    printf("\n %s \n", case_name);
    print_table(&t, "Исходная таблица:");
    insertion_sort(&t);
    print_table(&t, "Отсортированная таблица:");

    printf("\nВводите ключи для двоичного поиска (пустая строка = конец):\n");
    while (1) {
        int idx;
        printf("KEY> ");
        if (!read_line(key, sizeof(key))) break;
        if (key[0] == '\0') break;

        idx = binary_search_key(&t, key);
        if (idx >= 0) {
            printf("Найдено: индекс=%d, KEY=\"%s\", DATA=\"%s\"\n", idx, t.keys[idx], t.vals[idx]);
        } else {
            printf("Не найдено: KEY=\"%s\"\n", key);
        }
    }
}

int main(int argc, char **argv) {
    Table base;
    Table tmpSorted;
    FILE *in = stdin;

    if (argc >= 2) {
        in = fopen(argv[1], "r");
        if (!in) {
            printf("Ошибка: не удалось открыть файл: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("Использование: %s <файл_с_таблицей>\n", argv[0]);
        return 1;
    }

    if (!read_table_from(in, &base)) {
        printf("\nЗавершение: ошибка ввода таблицы.\n");
        fclose(in);
        return 1;
    }
    fclose(in);  // Закрываем файл после чтения таблицы

    /* Готовим вариант "уже отсортировано" */
    copy_table(&tmpSorted, &base);
    insertion_sort(&tmpSorted);

    /* Три требуемых случая - теперь ключи читаются с клавиатуры */
    sort_and_search(&tmpSorted, NULL, "Случай 1: уже упорядочено (с самого начала)");
    sort_and_search(&tmpSorted, reverse_table, "Случай 2: обратный порядок");
    sort_and_search(&base, NULL, "Случай 3: не упорядочено (как во входном файле)");
    return 0;
}
