#include <stdio.h>
#include <stdbool.h>

// Структура узла двоичного дерева 
struct Node {
    int data;           
    struct Node* left;  
    struct Node* right; 
};


// Функция создания нового узла
struct Node* createNode(int value) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    
    if (newNode == NULL) {
        printf("Ошибка выделения памяти!\n");
        return NULL;
    }
    
    newNode->data = value;
    newNode->left = NULL;
    newNode->right = NULL;
    
    return newNode;
}


// Функция вставки узла
struct Node* insertNode(struct Node* root, int value){
    if (root == NULL) {
        return createNode(value);
    }
    
    if (value < root->data) {
        root->left = insertNode(root->left, value);
    } 
    else {
        root->right = insertNode(root->right, value);
    }
    
    return root;
}


// Функция для проверки зеркальности деревьев
bool isMirror(struct Node* leftTree, struct Node* rightTree){
    if (leftTree == NULL && rightTree == NULL){
        return true;
    }
    
    if (leftTree == NULL || rightTree == NULL){
        return false;
    }
    
    return (leftTree->data == rightTree->data) &&           
           isMirror(leftTree->left, rightTree->right) &&    
           isMirror(leftTree->right, rightTree->left);      
}


// Функция для проверки симметричности всего дерева
bool isSymmetric(struct Node* root){
    if (root == NULL){
        return true;
    }
    
    return isMirror(root->left, root->right);
}


// Функция для визуализации дерева (с отступами)
void printTree(struct Node* root, int level) {
    if (root == NULL) {
        return;
    }
    
    printTree(root->right, level + 1);
    
    for (int i = 0; i < level; i++) {
        printf("    "); 
    }
    printf("%d\n", root->data);
    
    printTree(root->left, level + 1);
}


//Функция для нахождения миинимального
struct Node* findMin(struct Node* root){
    struct Node* current = root;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}


// Функция для удаления узла дерева 
struct Node* deleteNode(struct Node* root, int value){
    if (root == NULL){
        return root;
    }
    
    
    if (value < root->data) {
        root->left = deleteNode(root->left, value);
    } 
    else if (value > root->data) {
        root->right = deleteNode(root->right, value);
    } 
    else {
        // Нет левого поддерева
        if (root->left == NULL) {
            struct Node* temp = root->right;
            free(root);
            return temp;
        }
        // Нет правого поддерева
        else if (root->right == NULL) {
            struct Node* temp = root->left;
            free(root);
            return temp;
        }
        else{
            // Есть оба поддерева
            struct Node* temp = findMin(root->right);
            root->data = temp->data;
            root->right = deleteNode(root->right, temp->data);
        }
    }
    return root;
}


// Функция для демонстрации симметричного дерева
void createSymmetric(struct Node** root) {
    *root = createNode(1);
    (*root)->left = createNode(2);
    (*root)->right = createNode(2);
    (*root)->left->left = createNode(3);
    (*root)->left->right = createNode(4);
    (*root)->right->left = createNode(4);
    (*root)->right->right = createNode(3);
}

// Функция для демонстрации НЕсимметричного дерева
void createAsymmetric(struct Node** root) {
    *root = createNode(1);
    (*root)->left = createNode(2);
    (*root)->right = createNode(2);
    (*root)->left->right = createNode(3);
    (*root)->right->right = createNode(3);
}


// Функция для удяления всего дерева
void freeTree(struct Node* root){
    if (root == NULL){
        return root;
    }
    
    freeTree(root->left);
    freeTree(root->right);
    
    free(root);
}


int main() {
	struct Node* root = NULL;
	int choice, value;
	bool symmetric;
	
	while(1) {
	    printf("Команды\n");
	    printf("1. Добавить узел\n");
        printf("2. Показать дерево\n");
        printf("3. Удалить узел\n");
        printf("4. Проверить симметричность\n");
        printf("5. Создать симметричный пример\n");
        printf("6. Создать несимметричный пример\n");
        printf("7. Очистить дерево\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                printf("Введите значение для добавления: ");
                scanf("%d", &value);
                root = insertNode(root, value);
                printf("Узел %d добавлен\n", value);
                break;
                
            case 2:
                if (root == NULL) {
                    printf("Пустое дерево\n");
                } 
                else {
                    printf("Визуализация дерева (над визуализацие подумать):\n");
                    printTree(root, 0);
                }
                break;
                
            case 3:
                if (root == NULL) {
                    printf("Пустое дерево\n");
                } 
                else {
                    printf("Введите значение для удаления: ");
                    scanf("%d", &value);
                    root = deleteNode(root, value);
                    printf("Узел %d удален\n", value);
                }
                break;
                
            case 4:
                symmetric = isSymmetric(root);
                if (symmetric) {
                    printf("СИММЕТРИЧНО\n");
                } 
                else {
                    printf("НЕ симметрично\n");
                }
                break;
            
            case 5:
                if (root != NULL){
                    freeTree(root);
                }
                
                createSymmetric(&root);
                printf("Создан симметричное дерево\n");
                break;
                
            case 6:
                if (root != NULL){
                    freeTree(root);
                }
                
                createAsymmetric(&root);
                printf("Создано ассиметричное дерево\n");
                break;
                
            case 7:
                if (root != NULL) {
                    freeTree(root);
                    root = NULL;
                    printf("Дерево пустое\n");
                } else {
                    printf("Дерево пустое\n");
                }
                break;
                
            case 0:
                if (root != NULL) {
                    freeTree(root);
                }
                printf("\nПрограмма завершена\n");
                return 0;
                
            default:
                printf("Такой команды нет. Введите еще раз\n");
        }
        
	}

}



