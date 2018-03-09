com = g++ -std=c++11 -O2

build : main.o joint.o thulac_seg.o corpus.o public.o
	$(com) -l boost_system -l boost_filesystem -l boost_locale -o build main.o joint.o thulac_seg.o corpus.o public.o
main.o : main.cpp joint.h corpus.h public.h thulac_seg.h
	$(com) -g -c main.cpp
joint.o : joint.h joint.cpp
	$(com) -g -c joint.cpp
thulac_seg.o : thulac_seg.h thulac_seg.cpp
	$(com) -g -c thulac_seg.cpp
corpus.o : corpus.h corpus.cpp public.h
	$(com) -g -c corpus.cpp
public.o : public.h public.cpp 
	$(com) -g -c public.cpp
clean :
	rm main.o joint.o thulac_seg.o corpus.o public.o build  
