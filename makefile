devdoc.so: devdoc.cpp
	g++ -shared devdoc.cpp -fPIC -std=c++17 $(CXXFLAGS) -o devdoc.so -ldocview `pkg-config --cflags --libs libxml++-2.6` $(LDFLAGS)
