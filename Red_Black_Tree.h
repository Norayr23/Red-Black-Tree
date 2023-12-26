#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <iostream>
#include <initializer_list>
#include <queue>

template <typename T>
class Red_Black_Tree {
    enum Color {
        Red,
        Black
    };
    struct Node {
        Node() : left{&Nil}, right{&Nil}, parent{&Nil}, color(Red), value{} {}
        explicit Node(const T& v) : left{&Nil}, right{&Nil}, parent{&Nil}, color(Red), value{v} {}
        Node(Node* l, Node* r, Node* p, Color c, const T& v) : left{l}, right{r}, parent{p}, color(c), value{v} {}
        Node* left;
        Node* right;
        Node* parent;
        Color color;
        T value;
    };
public:
    Red_Black_Tree() : m_size{}, m_root{nil} {
    }
    Red_Black_Tree(const std::initializer_list<T>& list) : Red_Black_Tree() {
        for (const auto& el : list) {
            insert(el);
        }
    }
    Red_Black_Tree(const Red_Black_Tree& other) : Red_Black_Tree() {
        auto copy = [this](const T& value) { this->insert(value); };
        other.levelorderTraverse(copy);
    }
    Red_Black_Tree(Red_Black_Tree&& other) : Red_Black_Tree() {
        swap(other);
    }
    Red_Black_Tree& operator=(const Red_Black_Tree& rhs) {
        Red_Black_Tree tmp{rhs};
        swap(tmp);
        return *this;
    }
    Red_Black_Tree& operator=(Red_Black_Tree&& rhs) {
        if (&rhs == this) {
            return *this;
        }
        swap(rhs);
        rhs.clear();
        return *this;
    }
    void swap(Red_Black_Tree& other) noexcept {
        std::swap(m_size, other.m_size);
        std::swap(m_root, other.m_root);
    }
    void insert(const T& value) {
        Node* current = m_root;
        Node* parent = nil;
        while (current != nil) {
            parent = current;
            if (value < current->value) {
                current = current->left;
            }
            else if (value > current->value) {
                current = current->right;
            }
            else {
                return;
            }
        }
        Node* newNode = new Node(value);
        newNode->parent = parent;
        if (parent == nil) {
            m_root = newNode;
        }
        else if (value < parent->value) {
            parent->left = newNode;
        }
        else {
            parent->right = newNode;
        }
        insertFixup(newNode);
        ++m_size;
    }
    void remove(const T& value) {
        Node* node = getNode(value, m_root);
        if (node == nil) {
            return;
        }
        Color origColor = node->color;
        Node* problemNode = nil;
        if (node->left == nil) {
            problemNode = node->right;
            transplant(node, problemNode);
        }
        else if (node->right == nil) {
            problemNode = node->left;
            transplant(node, problemNode);
        }
        else {
            Node* successor = getMin(node->right);
            origColor = successor->color;
            problemNode = successor->right;
            if (successor != node->right) {
                transplant(successor, problemNode);
                successor->right = node->right;
                successor->right->parent = successor;
            }
            else {
                problemNode->parent = successor;
            }
            transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
            successor->color = node->color;
        }
        if (origColor == Black) {
            removeFixup(problemNode);
        }
        delete node;
        --m_size;
    }
    void clear() {
        doClear(m_root);
        m_root = nil;
        m_size = 0;
    }
    template <typename F>
    void inorderTraverse(F f) const {
        doInorder(f, m_root);
    }
    template <typename F>
    void preorderTraverse(F f) const {
        doPreorder(f, m_root);
    }
    template <typename F>
    void postorderTraverse(F f) const {
        doPostorder(f, m_root);
    }
    template <typename F>
    void levelorderTraverse(F f) const {
        doLevelorder(f, m_root);
    }
    const T& max() const {
        if (empty()) {
            throw std::logic_error("Read_Black_Tree is empty. Fail to get max value");
        }
        return getMax(m_root)->value;
    }
    const T& min() const {
        if (empty()) {
            throw std::logic_error("Read_Black_Tree is empty. Fail to get min value");
        }
        return getMin(m_root)->value;
    }
    bool search(const T& value) const { return getNode(value, m_root) != nil; }
    constexpr size_t size() const { return m_size; }
    constexpr bool empty() const { return !m_size; }
    void print() const {
          if (m_root == nil) {
            return;
        }
        std::queue<Node*> queue;
        queue.push(m_root);
        while (!queue.empty()) {
            auto tmp = queue.front();
            queue.pop();
            std::cout << tmp->value << "(" << (tmp->color == Red ? "R" : "B") << "), ";
            if (tmp->left != nil) {
                queue.push(tmp->left);
            }
            if (tmp->right != nil) {
                queue.push(tmp->right);
            }
        }
        std::cout << std::endl;
    }
    ~Red_Black_Tree() { clear(); }
private:
    Node* leftRotate(Node* node) {
        if (node == nil) {
            return nil;
        }
        Node* right = node->right;
        node->right = right->left;
        if (node->right != nil) {
            node->right->parent = node;
        }
        right->parent = node->parent;
        if (right->parent == nil) {
            m_root = right;
        }
        else if (node == right->parent->left) {
            right->parent->left = right;
        }
        else {
            right->parent->right = right;
        }
        right->left = node;
        node->parent = right;
        return right;   
    }
    Node* rightRotate(Node* node) {
        Node* left = node->left;
        node->left = left->right;
        if (node->left != nil) {
            node->left->parent = node;
        }
        left->parent = node->parent;
        if (left->parent == nil) {
            m_root = left;
        }
        else if (node == node->parent->left) {
            node->parent->left = left;
        }
        else {
            node->parent->right = left;
        }
        node->parent = left;
        left->right = node;
        return left;
    }
    void insertFixup(Node* node) {
        if (!node || node == nil) {
            return;
        }
        while (node->parent->color == Red) {
            if (node->parent == node->parent->parent->left) {
                Node* uncle = node->parent->parent->right;
                if (uncle->color == Red){
                    node->parent->color = Black;
                    uncle->color = Black;
                    node->parent->parent->color = Red;
                    node = node->parent->parent;
                }
                else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        leftRotate(node);
                    }
                    node->parent->color = Black;
                    node->parent->parent->color = Red;
                    rightRotate(node->parent->parent);
                }
            }
            else {
                Node* uncle = node->parent->parent->left;
                if (uncle->color == Red) {
                    node->parent->color = Black;
                    uncle->color = Black;
                    node->parent->parent->color = Red;
                    node = node->parent->parent;
                }
                else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rightRotate(node);
                    }
                    node->parent->color = Black;
                    node->parent->parent->color = Red;
                    leftRotate(node->parent->parent);
                }
            }
        }
        m_root->color = Black;
    }
    void transplant(Node* dest, Node* target) {
        if (dest == nil) {
            return;
        }
        if (dest->parent == nil) {
            m_root = target;
        }
        else if (dest == dest->parent->left) {
            dest->parent->left = target;
        }
        else {
            dest->parent->right = target;
        }
        target->parent = dest->parent;
    }
    void removeFixup(Node* node) {
        while (node != m_root && node->color == Black) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                if (sibling->color == Red) {
                    sibling->color = Black;
                    node->parent->color = Red;
                    leftRotate(node->parent);
                    sibling = node->parent->right;
                }
                if (sibling->left->color == Black && sibling->right->color == Black) {
                    sibling->color = Red;
                    node = node->parent;
                }
                else {
                    if (sibling->right->color == Black) {
                        sibling->left->color = Black;
                        sibling->color = Red;
                        rightRotate(sibling);
                        sibling = node->parent->right;
                    }
                    sibling->color = node->parent->color;
                    node->parent->color = Black;
                    sibling->right->color = Black;
                    leftRotate(node->parent);
                    node = m_root;
                }
            }
            else {
                Node* sibling = node->parent->left;
                if (sibling->color == Red) {
                    sibling->color = Black;
                    node->parent->color = Red;
                    rightRotate(node->parent);
                    sibling = node->parent->left;
                }
                if (sibling->right->color == Black && sibling->left->color == Black) {
                    sibling->color = Red;
                    node = node->parent;
                }
                else {
                    if (sibling->left->color == Black) {
                        sibling->right->color = Black;
                        sibling->color = Red;
                        leftRotate(sibling);
                        sibling = node->parent->left;
                    }
                    sibling->color = node->parent->color;
                    node->parent->color = Black;
                    sibling->left->color = Black;
                    rightRotate(node->parent);
                    node = m_root;
                }
            }
        }
        node->color = Black;
    }
    Node* nil = &Nil;
    Node* m_root;
    size_t m_size;
private:
    template <typename F>
    static void doInorder(F f, Node* node) {
        if (node == &Nil) {
            return;
        }
        doInorder(f, node->left);
        f(node->value);
        doInorder(f, node->right);
    }
    template <typename F>
    static void doPreorder(F f, Node* node) {
        if (node == &Nil) {
            return;
        }
        f(node->value);
        doPreorder(f, node->left);
        doPreorder(f, node->right);
    }
    template <typename F>
    static void doPostorder(F f, Node* node) {
        if (node == &Nil) {
            return;
        }
        doPostorder(f, node->left);
        doPostorder(f, node->right);
        f(node->value);
    }
    template <typename F>
    static void doLevelorder(F f, Node* node) { 
        if (node == &Nil) {
            return;
        }
        std::queue<Node*> queue;
        queue.push(node);
        while (!queue.empty()) {
            auto tmp = queue.front();
            queue.pop();
            f(tmp->value);
            if (tmp->left != &Nil) {
                queue.push(tmp->left);
            }
            if (tmp->right != &Nil) {
                queue.push(tmp->right);
            }
        }
    }
    static Node* getMax(Node* node) {
        if (node == &Nil) {
            return &Nil;
        }
        while (node->right != &Nil) {
            node = node->right;
        }
        return node;
    }
    static Node* getMin(Node* node) {
        if (node == &Nil) {
            return &Nil;
        }
        while (node->left != &Nil) {
            node = node->left;
        }
        return node;
    }
    static Node* getNode(const T& value, Node* node) {
        while (node != &Nil && node->value != value) {
            if (value > node->value) {
                node = node->right;
            }
            else {
                node = node->left;
            }
        }
        return node;
    }
    static void doClear(Node* node) {
        if (!node || node == &Nil) {
            return;
        }
        doClear(node->left);
        doClear(node->right);
        delete node;
    }    
    static Node Nil;
};
template <typename T>
typename Red_Black_Tree<T>::Node Red_Black_Tree<T>::Nil = Red_Black_Tree<T>::Node(nullptr, nullptr, nullptr, Red_Black_Tree<T>::Black, T{});

#endif
