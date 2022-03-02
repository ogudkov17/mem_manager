test: mem_manager.o test/test01.cpp
	g++ test/test01.cpp mem_manager.o /usr/src/gtest/src/gtest-all.cc -lpthread -L/usr/local/lib -I/usr/src/gtest -o test/test01

mem_manager.o: mem_manager.cpp
	g++ -c mem_manager.cpp


clean:
	rm -rf *.o
	rm -rf memory_manager_test
	rm -rf test/test01


