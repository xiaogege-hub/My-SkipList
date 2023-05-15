#include <iostream>
#include "skiplist.h"

int main() {
    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    Skiplist<int, std::string> skiplist(4);
    skiplist.insert_element(1, "蔚来"); 
	skiplist.insert_element(3, "小鹏"); 
	skiplist.insert_element(7, "理想"); 
	skiplist.insert_element(8, "特斯拉"); 
	skiplist.insert_element(9, "比亚迪"); 
	skiplist.insert_element(19, "大众"); 
    skiplist.insert_element(19, "BBA");
    std::cout << "skipList size:" << skiplist.size() << std::endl;

    skiplist.save_file();

    skiplist.search_element(9);
    skiplist.search_element(18);

    skiplist.display_list();
    skiplist.delete_element(3);
    skiplist.delete_element(7);
    skiplist.display_list();

    std::cout << "skipList size:" << skiplist.size() << std::endl;
}

/*
int main () {
    //测试load_file()
    Skiplist<int, std::string> skiplist(4);
    std::cout << "skipList size:" << skiplist.size() << std::endl;
    skiplist.load_file();
    std::cout << "skipList size:" << skiplist.size() << std::endl;
    skiplist.display_list();
}
*/