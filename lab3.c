#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR 256

typedef enum {
    NODE_NUMBER,    // число
    NODE_OPERATOR   // оператор
} NodeType;

typedef struct Node {
    NodeType type;
    char op;                    // оператор (+, -, *, /)
    double value;               // значение для чисел
    struct Node* left;          // левый потомок
    struct Node* right;         // правый потомок
} Node;

typedef struct Stack {
    Node* items[MAX_EXPR];
    int top;
} Stack;

// Стек для операторов
typedef struct OpStack {
    int items[MAX_EXPR];
    int top;
} OpStack;

// Инициализация стека узлов
void init_stack(Stack* s) {
    s->top = -1;
}

void push(Stack* s, Node* node) {
    if (s->top < MAX_EXPR - 1) {
        s->items[++(s->top)] = node;
    }
}

Node* pop(Stack* s) {
    if (s->top >= 0) {
        return s->items[(s->top)--];
    }
    return NULL;
}

int is_empty(Stack* s) {
    return s->top == -1;
}

// Инициализация стека операторов
void init_op_stack(OpStack* s) {
    s->top = -1;
}

void push_op(OpStack* s, int val) {
    if (s->top < MAX_EXPR - 1) {
        s->items[++(s->top)] = val;
    }
}

int pop_op(OpStack* s) {
    if (s->top >= 0) {
        return s->items[(s->top)--];
    }
    return 0;
}

int is_op_empty(OpStack* s) {
    return s->top == -1;
}

int is_zero(Node* node) {
    if (node == NULL) return 0;
    if (node->type == NODE_NUMBER && node->value == 0.0) return 1;
    return 0; 
}

int get_priority(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return 0;
    }
}

void free_tree(Node* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

Node* create_number_node(double a) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    node->type = NODE_NUMBER;
    node->value = a;
    node->op = '\0';
    node->left = node->right = NULL;
    return node;
}

Node* create_operator_node(char op, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    node->type = NODE_OPERATOR;
    node->op = op;
    node->value = 0;
    node->left = left;
    node->right = right;
    return node;
}

// Упрощение дерева
Node* simplify(Node* root) {
    if (root == NULL) return NULL;

    root->left = simplify(root->left);
    root->right = simplify(root->right);

    if (root->type != NODE_OPERATOR) return root;
    
    if (root->op == '*') {
        if (is_zero(root->left)) {
            free_tree(root->right);
            Node* result = create_number_node(0.0);
            free(root);
            return result;
        }
        if (is_zero(root->right)) {
            free_tree(root->left);
            Node* result = create_number_node(0.0);
            free(root);
            return result;
        }
    }
    
    // Дополнительное упрощение: если выражение вида 0 + x, то x
    if (root->op == '+') {
        if (is_zero(root->left)) {
            Node* result = root->right;
            free(root->left);
            free(root);
            return result;
        }
        if (is_zero(root->right)) {
            Node* result = root->left;
            free(root->right);
            free(root);
            return result;
        }
    }
    
    // Если выражение вида x - 0, то x
    if (root->op == '-') {
        if (is_zero(root->right)) {
            Node* result = root->left;
            free(root->right);
            free(root);
            return result;
        }
    }
    
    return root;
}

void print_expr(Node* root) {
    if (root == NULL) return;
    
    if (root->type == NODE_NUMBER) {
        if (root->value == (int)root->value) {
            printf("%d", (int)root->value);
        } else {
            printf("%g", root->value);
        }
    } 
    else {
        int need_paren = 0;
        
        if (root->left && root->left->type == NODE_OPERATOR) {
            int left_priority = get_priority(root->left->op);
            int curr_priority = get_priority(root->op);
            if (left_priority < curr_priority) need_paren = 1;
        }
        
        if (need_paren) printf("(");
        print_expr(root->left);
        if (need_paren) printf(")");
        
        printf(" %c ", root->op);
        
        need_paren = 0;
        if (root->right && root->right->type == NODE_OPERATOR) {
            int right_priority = get_priority(root->right->op);
            int curr_priority = get_priority(root->op);
            if (right_priority <= curr_priority) need_paren = 1;
        }
        
        if (need_paren) printf("(");
        print_expr(root->right);
        if (need_paren) printf(")");
    }
}

// Проверка, является ли символ оператором
int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

// Преобразование выражения в дерево
Node* parse_expression(const char* expr) {
    Stack operand_stack;
    OpStack operator_stack;
    
    init_stack(&operand_stack);
    init_op_stack(&operator_stack);
    
    int i = 0;
    int len = strlen(expr);
    int last_was_operator = 1;  // для обработки унарного минуса
    
    while (i < len) {
        // Пропускаем пробелы
        if (isspace(expr[i])) {
            i++;
            continue;
        }
        
        // Число
        if (isdigit(expr[i]) || (expr[i] == '.')) {
            char* endptr;
            double val = strtod(expr + i, &endptr);
            push(&operand_stack, create_number_node(val));
            i = endptr - expr;
            last_was_operator = 0;
            continue;
        }
        
        // Унарный минус (отрицательное число)
        if (expr[i] == '-' && last_was_operator) {
            i++;  // пропускаем минус
            // читаем число после минуса
            char* endptr;
            double val = -strtod(expr + i, &endptr);
            push(&operand_stack, create_number_node(val));
            i = endptr - expr;
            last_was_operator = 0;
            continue;
        }
        
        // Переменная (буква)
        if (isalpha(expr[i])) {
            push(&operand_stack, create_number_node(1.0));
            i++;
            last_was_operator = 0;
            continue;
        }
        
        // Открывающая скобка
        if (expr[i] == '(') {
            push_op(&operator_stack, -1);  // -1 для '('
            i++;
            last_was_operator = 1;
            continue;
        }
        
        // Закрывающая скобка
        if (expr[i] == ')') {
            while (!is_op_empty(&operator_stack)) {
                int op = pop_op(&operator_stack);
                if (op == -1) break;  // встретили '('
                
                Node* right = pop(&operand_stack);
                Node* left = pop(&operand_stack);
                push(&operand_stack, create_operator_node((char)op, left, right));
            }
            i++;
            last_was_operator = 0;
            continue;
        }
        
        // Оператор
        if (is_operator(expr[i])) {
            char current_op = expr[i];
            int curr_priority = get_priority(current_op);
            
            while (!is_op_empty(&operator_stack)) {
                int top = pop_op(&operator_stack);
                if (top == -1) {  // это '('
                    push_op(&operator_stack, top);
                    break;
                }
                
                int top_priority = get_priority((char)top);
                if (top_priority >= curr_priority) {
                    Node* right = pop(&operand_stack);
                    Node* left = pop(&operand_stack);
                    push(&operand_stack, create_operator_node((char)top, left, right));
                } else {
                    push_op(&operator_stack, top);
                    break;
                }
            }
            push_op(&operator_stack, current_op);
            i++;
            last_was_operator = 1;
            continue;
        }
        
        i++;
    }
    
    // Выталкиваем оставшиеся операторы
    while (!is_op_empty(&operator_stack)) {
        int op = pop_op(&operator_stack);
        if (op == -1) continue;  // пропускаем '('
        
        Node* right = pop(&operand_stack);
        Node* left = pop(&operand_stack);
        push(&operand_stack, create_operator_node((char)op, left, right));
    }
    
    return pop(&operand_stack);
}

int main() {
    char expr[MAX_EXPR];
    Node* root = NULL;
    Node* simplified = NULL;

    printf("Поддерживаемые операции: +, -, *, /\n");
    printf("Введите 'quit' для выхода\n");
    
    while (1) {
        printf("Выражение> ");
        
        if (fgets(expr, MAX_EXPR, stdin) == NULL) {
            break;
        }
        
        expr[strcspn(expr, "\n")] = '\0';
        
        if (strcmp(expr, "quit") == 0) {
            printf("Выход...\n");
            break;
        }
        
        if (strlen(expr) == 0) {
            continue;
        }
        
        
        root = parse_expression(expr);
        if (root == NULL) {
            printf("Ошибка: не удалось разобрать выражение!\n");
            continue;
        }
        
        
        simplified = simplify(root);
        
        printf("Упрощенное выражение: ");
        print_expr(simplified);
        printf("\n\n");
        
        free_tree(simplified);
    }
    return 0;
}