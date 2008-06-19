CPPFLAGS=`wx-config --cxxflags` -I../tinfra/trunk
LDFLAGS=`wx-config --libs` -L../tinfra/trunk -ltinfra

dialogrunner: dialogrunner.o
	$(CXX)  $< -o $@ $(LDFLAGS)
	
.cpp.o:
	$(CXX) -g $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<
