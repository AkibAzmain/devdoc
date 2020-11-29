devdoc.so: devdoc.cpp
	g++ -shared devdoc.cpp -fPIC -std=c++17 $(CXXFLAGS) -o devdoc.so -ldocview $(LDFLAGS)
