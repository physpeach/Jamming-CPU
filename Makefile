test: testsrc/test.o cpp/MT.o cpp/conf.o testcpp/conf_test.o cpp/particles.o testcpp/particles_test.o
	g++ -o $@ $^

%.o: %.cpp
	g++ -o $@ -c $<
