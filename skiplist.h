#include <iostream>
#include <fstream>
#include <mutex>
#include <cstring>

#define STORE_FILE "store/saveFile"

std::mutex mtx;

//--------------------Class template to implement node---------------------
template<typename K, typename V>
class Node {
public:
    Node(K, V, int);
    ~Node();
    K get_key() const;
    V get_value() const;

public:
    //Linear array to hold pointers to next node of different level
    Node<K, V>** forward;
    int node_level;
private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;
    //level + 1, because array index is from 0 - level
    this->forward = new Node<K, V>*[level + 1];//new 一个指针数组
    // Fill forward array with 0(NULL) 
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const{
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const{
    return value;
}

//----------------------Class template for Skip list-------------------------
template<typename K, typename V>
class Skiplist{
public:
    Skiplist(int);
    ~Skiplist();
    int insert_element(K, V);    //插入数据
    bool delete_element(K);      //删除数据
    bool search_element(K);      //查询数据
    void save_file();
    void load_file();
    void display_list();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string& key, std::string& value);
    bool is_valid_string(const std::string& str);
    int get_random_level();

private:
    int _max_level;         //Maximum level of the skip list
    int _cur_level;         //current level of skip list 
    int _element_count;     //skiplist current element count
    Node<K, V>* _header;    //pointer to header node 
    
    //file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;
};

//construct skip list
template<typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level) {
    this->_max_level = max_level;
    this->_cur_level = 0;
    this->_element_count = 0;

    //create header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template<typename K, typename V> 
Skiplist<K, V>::~Skiplist() {
    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

// Insert given key and value in skip list 
// return -1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
*/
template<typename K, typename V>
int Skiplist<K, V>::insert_element(const K key, const V value) {
    mtx.lock();
    Node<K, V>* current = this->_header;

    //create update array and initialize it 
    //update is array which put node that the node->forward[i] should be operated later
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    //start form highest level of skip list 
    for (int i = _cur_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    //reached level 0 and let forward pointer to right node
    current = current->forward[0];

    //if current node have key equal to searched key, we return -1
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return -1;
    }

    // if current is NULL that means we have reached end of the level 
    // if current's key is not equal to key that means we have to insert node
    if (current == NULL || current->get_key() != key) {
        // Generate a random level for node  (很重要)
        //小林中也说到 跳表在创建节点的时候，随机生成每个节点的层数 维持相邻两层的节点数量的比例为 2 : 1 
        int random_level = get_random_level();

        //If random level is greater than skip list's current level, initialize update value with pointer to header
        if (random_level > _cur_level) {
            for (int i = _cur_level + 1; i <= random_level; i++) {
                update[i] = _header;
            }
            _cur_level = random_level;
        }

        //create new node with random level generated 
        Node<K, V>* insert_node = new Node<K, V>(key, value, random_level);

        //insert node 
        for (int i = 0; i <= random_level; i++) {
            insert_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insert_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;   
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

//Delete element from skip list 
template<typename K, typename V> 
bool Skiplist<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    // start from highest level of skip list
    for (int i = _cur_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // reached level 0 and let forward pointer to right node
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current != NULL && current->get_key() == key) {
        // start from the lowest level and delete the current node of each level
        for (int i = 0; i <= _cur_level; i++) {
            // if at level i, next node is not target node, break the loop
            if (update[i]->forward[i] != current)
                break;
            // delete nodes （但是这里并没有真正释放内存）
            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        while (_cur_level > 0 && _header->forward[_cur_level] == NULL) {
            _cur_level--;
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        _element_count --;
        mtx.unlock();
        return true;
    }
    std::cout << "fail to delete key: "<< key << ", not exists" << std::endl;
    mtx.unlock();
    return false;
}


//Search for element in skip list
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V> 
bool Skiplist<K, V>::search_element(K key) {
    std::cout << "--------------search_element--------------" << std::endl;
    Node<K, V>* current = this->_header;

    //1.start from highest level of skip list
    for (int i = _cur_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //2.reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    //3.if current node have key equal to searched key, we get it
    if (current != NULL && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// save data in memory to file 存储
template<typename K, typename V> 
void Skiplist<K, V>::save_file() {
    std::cout << "--------save file--------" << std::endl;
    _file_writer.open(STORE_FILE);
    
    //直接指向skiplist的最底层，因为最底层包含所有数据
    Node<K, V>* node = this->_header->forward[0];
    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << "\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

// Load data from disk
template<typename K, typename V> 
void Skiplist<K, V>::load_file() {
    std::cout << "--------load_file--------" << std::endl;
    _file_reader.open(STORE_FILE);
    std::string line;
    std::string key = "";
    std::string value = "";
    while (getline(_file_reader, line)) {//一行一行读数据
        get_key_value_from_string(line, key, value);
        if (key.empty() || value.empty()) {
            continue;
        }
        insert_element(atoi(key.c_str()), value);
        std::cout << "key:" << key << " value:" << value << std::endl;
    }
    _file_reader.close();
    return;
}

//Display skip list 
template<typename K, typename V> 
void Skiplist<K, V>::display_list() {
    std::cout << "\n********Skip List********\n";
    //从下往上，一层层打印
    for (int i = 0; i <= _cur_level; i++) {
        Node<K, V>* node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << "; ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Get current SkipList size 
template<typename K, typename V> 
int Skiplist<K, V>::size() {
    return _element_count;
}

template<typename K, typename V>
void Skiplist<K, V>::get_key_value_from_string(const std::string& str, std::string& key, std::string& value) {
    if (!is_valid_string(str)) {
        return;
    }
    int len = str.find(":");
    key = str.substr(0, len);
    value = str.substr(len + 1);
}

template<typename K, typename V>
bool Skiplist<K, V>::is_valid_string(const std::string& str) {
    if (str.empty()) return false;
    if (str.find(":") == std::string::npos) {
        return false;
    }
    return true;
}

template<typename K, typename V>
int Skiplist<K, V>::get_random_level(){
    int k = 1;
    while (rand() % 2) { //模拟抛硬币 
        k++;
    }
    k = (k < _max_level) ? k : _max_level;//当然，最大也不能超过 _max_level
    return k;
}