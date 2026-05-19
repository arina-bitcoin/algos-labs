#include "btree.h"

BTree* btree_create(void) {
    BTree *tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) { perror("malloc BTree"); exit(EXIT_FAILURE); }
    tree->t    = T;
    tree->root = NULL;
    return tree;
}

Node* create_node(int leaf) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (!node) { perror("malloc Node"); exit(EXIT_FAILURE); }
    node->leaf = leaf;
    node->n    = 0;
    for (int i = 0; i < MAX_CHILDREN; i++)
        node->children[i] = NULL;
    return node;
}

static int find_key_index(Node *node, int key) {
    int i = 0;
    while (i < node->n && node->keys[i] < key)
        i++;
    return i;
}

void btree_split_child(Node *parent, int index, Node *child) {
    Node *new_node = create_node(child->leaf);
    new_node->n = MIN_KEYS_NON_ROOT;

    // Копируем ключи и значения
    for (int i = 0; i < MIN_KEYS_NON_ROOT; i++) {
        new_node->keys[i] = child->keys[i + T];
        strcpy(new_node->values[i], child->values[i + T]);
    }

    if (!child->leaf) {
        for (int i = 0; i < T; i++)
            new_node->children[i] = child->children[i + T];
    }

    child->n = MIN_KEYS_NON_ROOT;

    for (int i = parent->n; i >= index + 1; i--)
        parent->children[i + 1] = parent->children[i];
    parent->children[index + 1] = new_node;

    for (int i = parent->n - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
        strcpy(parent->values[i + 1], parent->values[i]);
    }
    parent->keys[index] = child->keys[MIN_KEYS_NON_ROOT];
    strcpy(parent->values[index], child->values[MIN_KEYS_NON_ROOT]);
    parent->n++;
}

void btree_insert_nonfull(Node *node, int key, const char *value) {
    int i = node->n - 1;

    if (node->leaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            strcpy(node->values[i + 1], node->values[i]);
            i--;
        }
        node->keys[i + 1] = key;
        strcpy(node->values[i + 1], value);
        node->n++;
    } else {
        while (i >= 0 && node->keys[i] > key)
            i--;
        i++;

        if (node->children[i]->n == MAX_KEYS) {
            btree_split_child(node, i, node->children[i]);
            if (key > node->keys[i])
                i++;
        }
        btree_insert_nonfull(node->children[i], key, value);
    }
}

void btree_insert(BTree *tree, int key, const char *value) {
    if (!tree->root) {
        tree->root = create_node(1);
        tree->root->keys[0] = key;
        strcpy(tree->root->values[0], value);
        tree->root->n = 1;
        return;
    }

    if (tree->root->n == MAX_KEYS) {
        Node *new_root = create_node(0);
        new_root->children[0] = tree->root;
        btree_split_child(new_root, 0, tree->root);
        int i = (key > new_root->keys[0]) ? 1 : 0;
        btree_insert_nonfull(new_root->children[i], key, value);
        tree->root = new_root;
    } else {
        btree_insert_nonfull(tree->root, key, value);
    }
}

int btree_get_predecessor(Node *node, char *value) {
    while (!node->leaf)
        node = node->children[node->n];
    strcpy(value, node->values[node->n - 1]);
    return node->keys[node->n - 1];
}

int btree_get_successor(Node *node, char *value) {
    while (!node->leaf)
        node = node->children[0];
    strcpy(value, node->values[0]);
    return node->keys[0];
}

void btree_borrow_from_left(Node *parent, int index) {
    Node *child = parent->children[index];
    Node *left  = parent->children[index - 1];

    for (int i = child->n - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        strcpy(child->values[i + 1], child->values[i]);
    }
    if (!child->leaf) {
        for (int i = child->n; i >= 0; i--)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = parent->keys[index - 1];
    strcpy(child->values[0], parent->values[index - 1]);
    
    parent->keys[index - 1] = left->keys[left->n - 1];
    strcpy(parent->values[index - 1], left->values[left->n - 1]);

    if (!child->leaf)
        child->children[0] = left->children[left->n];

    child->n++;
    left->n--;
}

void btree_borrow_from_right(Node *parent, int index) {
    Node *child = parent->children[index];
    Node *right = parent->children[index + 1];

    child->keys[child->n] = parent->keys[index];
    strcpy(child->values[child->n], parent->values[index]);
    
    parent->keys[index] = right->keys[0];
    strcpy(parent->values[index], right->values[0]);

    if (!child->leaf)
        child->children[child->n + 1] = right->children[0];

    for (int i = 0; i < right->n - 1; i++) {
        right->keys[i] = right->keys[i + 1];
        strcpy(right->values[i], right->values[i + 1]);
    }
    if (!right->leaf) {
        for (int i = 0; i < right->n; i++)
            right->children[i] = right->children[i + 1];
    }

    child->n++;
    right->n--;
}

void btree_merge(Node *parent, int index) {
    Node *child = parent->children[index];
    Node *right = parent->children[index + 1];

    child->keys[MIN_KEYS_NON_ROOT] = parent->keys[index];
    strcpy(child->values[MIN_KEYS_NON_ROOT], parent->values[index]);

    for (int i = 0; i < right->n; i++) {
        child->keys[i + MIN_KEYS_NON_ROOT + 1] = right->keys[i];
        strcpy(child->values[i + MIN_KEYS_NON_ROOT + 1], right->values[i]);
    }
    if (!child->leaf) {
        for (int i = 0; i <= right->n; i++)
            child->children[i + MIN_KEYS_NON_ROOT + 1] = right->children[i];
    }
    child->n += right->n + 1;

    for (int i = index; i < parent->n - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        strcpy(parent->values[i], parent->values[i + 1]);
        parent->children[i + 1] = parent->children[i + 2];
    }
    parent->n--;

    free(right);
}

void btree_fix_child(Node *parent, int index) {
    if (index > 0 && parent->children[index - 1]->n > MIN_KEYS_NON_ROOT)
        btree_borrow_from_left(parent, index);
    else if (index < parent->n && parent->children[index + 1]->n > MIN_KEYS_NON_ROOT)
        btree_borrow_from_right(parent, index);
    else {
        if (index < parent->n)
            btree_merge(parent, index);
        else
            btree_merge(parent, index - 1);
    }
}

void btree_delete_internal(Node *node, int key) {
    int i = find_key_index(node, key);

    if (i < node->n && node->keys[i] == key) {
        if (node->leaf) {
            for (int j = i; j < node->n - 1; j++) {
                node->keys[j] = node->keys[j + 1];
                strcpy(node->values[j], node->values[j + 1]);
            }
            node->n--;
        } else {
            Node *left_child  = node->children[i];
            Node *right_child = node->children[i + 1];
            char temp_value[MAX_VALUE_LEN];

            if (left_child->n > MIN_KEYS_NON_ROOT) {
                int pred = btree_get_predecessor(left_child, temp_value);
                node->keys[i] = pred;
                strcpy(node->values[i], temp_value);
                btree_delete_internal(left_child, pred);
            } else if (right_child->n > MIN_KEYS_NON_ROOT) {
                int succ = btree_get_successor(right_child, temp_value);
                node->keys[i] = succ;
                strcpy(node->values[i], temp_value);
                btree_delete_internal(right_child, succ);
            } else {
                btree_merge(node, i);
                btree_delete_internal(left_child, key);
            }
        }
    } else {
        if (node->leaf) return;

        int last = (i == node->n);

        if (node->children[i]->n == MIN_KEYS_NON_ROOT) {
            btree_fix_child(node, i);
            if (last && i > node->n)
                i--;
        }

        btree_delete_internal(node->children[i], key);
    }
}

void btree_delete(BTree *tree, int key) {
    if (!tree->root) return;

    btree_delete_internal(tree->root, key);

    if (tree->root->n == 0 && !tree->root->leaf) {
        Node *old_root = tree->root;
        tree->root = old_root->children[0];
        free(old_root);
    }
}

char* btree_search_node(Node *node, int key) {
    if (!node) return NULL;
    
    static char result[MAX_VALUE_LEN];
    int i = find_key_index(node, key);
    
    if (i < node->n && node->keys[i] == key) {
        strcpy(result, node->values[i]);
        return result;
    }
    
    if (node->leaf) return NULL;
    return btree_search_node(node->children[i], key);
}

char* btree_search(BTree *tree, int key) {
    return btree_search_node(tree->root, key);
}

void btree_print_node(Node *node, int level, FILE *output) {
    if (!node) return;
    for (int i = 0; i < level; i++) fprintf(output, "  ");
    fprintf(output, "[");
    for (int i = 0; i < node->n; i++) {
        fprintf(output, "%d:%s", node->keys[i], node->values[i]);
        if (i < node->n - 1) fprintf(output, " | ");
    }
    fprintf(output, "]\n");
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            btree_print_node(node->children[i], level + 1, output);
    }
}

void btree_print(BTree *tree, FILE *output) {
    if (!tree->root || tree->root->n == 0) { 
        fprintf(output, "  (дерево пусто)\n"); 
        return; 
    }
    btree_print_node(tree->root, 0, output);
}

void btree_free_node(Node *node) {
    if (!node) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            btree_free_node(node->children[i]);
    }
    free(node);
}

void btree_free(BTree *tree) {
    if (!tree) return;
    btree_free_node(tree->root);
    free(tree);
}