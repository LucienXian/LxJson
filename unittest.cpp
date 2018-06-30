#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#define JSON11_TEST_ASSERT(b) assert(b)
#define TEST_OBJ_AND_ARRAY test1()
#define TEST_PARSE_NULL test2("null", 1)
#define TEST_PARSE_BOOL1 test2("true", 2)
#define TEST_PARSE_BOOL2 test2("false", 2)
#define TEST_PARSE_STRING(str) test3(str)
#define TEST_PARSE_NUMBER test4()

using namespace lxjson;


void test1(){
    //test object & array
    std::string out_arr, out_obj;
    Json j_arr = Json(Json::array{Json(1), Json(2), Json(3)});
    j_arr.serialize(out_arr);
    Json j_obj(Json::object {
        { "key1",    Json("value1") },
        { "key2", Json(false) },
        { "key3", j_arr },
    });
    j_obj.serialize(out_obj);
    JSON11_TEST_ASSERT(out_arr == "[1, 2, 3]");
    std::cout << out_arr << std::endl;
    JSON11_TEST_ASSERT(out_obj=="{\"key1\": \"value1\", \"key2\": false, \"key3\": [1, 2, 3]}");
    std::cout << out_obj << std::endl;
}

void test2(std::string test_lit, int flag){
    //test parse
    std::string err_com;
    //std::string test_lit = "null";
    Json ret = Json::parse(test_lit, err_com);
    if (err_com.size()){
        std::cout << err_com << std::endl;
        return;
    }
    if (flag==1)
        JSON11_TEST_ASSERT(ret.is_null());
    if (flag==2)
        JSON11_TEST_ASSERT(ret.is_bool());
}

void test3(std::string test) {
    std::string err_com;
    Json ret = Json::parse(test, err_com);
    if (err_com.size()){
        std::cout << err_com << std::endl;
        return;
    }
    JSON11_TEST_ASSERT(ret.is_string());
    std::cout << ret.string_value().data() << std::endl;
}

void test4() {
    std::string err_com;
    //std::string test = R"(12E+2)";
    //std::string test = R"(12e-2)";
    //std::string test = R"(12.123)";
    std::string test = R"(120)";
    //std::string test = R"(012)"; error
    Json ret = Json::parse(test, err_com);
    if (err_com.size()){
        std::cout << err_com << std::endl;
        return;
    }
    JSON11_TEST_ASSERT(ret.is_number());
    std::cout << ret.double_value() << std::endl;
}


int main()
{
    Json js("asdwq1");
    JSON11_TEST_ASSERT(js.is_string());
    std::string out;
    std::map<std::string, Json> m1 { { "k1", js } };

    Json my_json(m1);
    my_json.serialize(out);
    JSON11_TEST_ASSERT(out=="{\"k1\": \"asdwq1\"}");

    /*
    TEST_OBJ_AND_ARRAY;
    TEST_PARSE_NULL;
    TEST_PARSE_BOOL1;
    TEST_PARSE_BOOL2;*/
    TEST_PARSE_NUMBER;

    //TEST_PARSE_STRING(R"("Hell\u00F6")");
    
    return 0;
}