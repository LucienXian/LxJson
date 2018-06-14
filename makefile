unittest: json.cpp json.hpp unittest.cpp
	$(CXX) $(CANARY_ARGS) -O -std=c++11 json.cpp unittest.cpp -o unittest -fno-rtti -fno-exceptions

clean:
	if [ -e unittest ]; then rm unittest; fi

.PHONY: clean