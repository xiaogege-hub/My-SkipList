#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "../skiplist.h"

#define THREAD_NUMS 1
#define TEST_COUNT 100000
Skiplist<int, std::string> skipList(18);

void insertElement() {
    std::cout << std::this_thread::get_id() << std::endl;
    int count = TEST_COUNT / THREAD_NUMS; 
    for (int i = 0; i < count; i++) {
		skipList.insert_element(rand() % TEST_COUNT, "a"); 
	}
}

void getElement() {
    std::cout << std::this_thread::get_id() << std::endl;
    int count = TEST_COUNT / THREAD_NUMS; 
    for (int i = 0; i < count; i++) {
		skipList.search_element(rand() % TEST_COUNT); 
	}
}

int main() {
    srand(time(NULL));
    {
        std::thread tid[THREAD_NUMS];
        int rc;

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < THREAD_NUMS; i++) {
            std::cout << "main() : creating thread, " << i << std::endl;
            tid[i] = std::thread(insertElement);
        }

        for (int i = 0; i < THREAD_NUMS; i++) {
            tid[i].join();
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "insert elapsed:" << elapsed.count() << std::endl;
    }
    sleep(2);
    skipList.display_list();
    skipList.save_file();

    return 0;
}
