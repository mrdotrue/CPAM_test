all: interval_graph

map_example:		map_example.cpp
	g++ -O3 -DNDEBUG -mcx16 -march=native -DHOMEGROWN -pthread -std=c++17 -Wall -I../../parlaylib/include -I../../include -o map_example map_example.cpp -L/usr/local/lib -ljemalloc


interval_graph: interval_graph.cpp interval_graph.h
	g++ -g -mcx16 -march=native -DHOMEGROWN -pthread -std=c++17 -Wall -I../../parlaylib/include -I../../include -o interval_graph interval_graph.cpp -L/usr/local/lib -ljemalloc

clean:
	rm -f test map_example  interval_graph
