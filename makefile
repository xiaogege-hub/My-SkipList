myskiplist: main.cpp
	g++ main.cpp -o ./main 
	rm -f ./*.o

clean:
	rm -f ./*.so
	rm ./main