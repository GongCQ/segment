build : main.o corpus.o public.o
	clang++ -l boost_system -l boost_filesystem -l boost_locale  -o build main.o corpus.o public.o
main.o : main.cpp corpus.h public.h
	clang++ -std=c++11 -O2 -g -c main.cpp
corpus.o : corpus.h corpus.cpp public.h
	clang++ -std=c++11 -O2 -g -c corpus.cpp
public.o : public.h public.cpp 
	clang++ -std=c++11 -O2 -g -c public.cpp
clean :
	rm main.o corpus.o public.o build  