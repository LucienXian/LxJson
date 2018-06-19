#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#define JSON11_TEST_ASSERT(b) assert(b)

using namespace lxjson;


int main()
{
    Json js("asdwq1");
    JSON11_TEST_ASSERT(js.is_string());
    std::string out;
    std::map<std::string, Json> m1 { { "k1", js } };

    Json my_json(m1);
    my_json.serialize(out);
    JSON11_TEST_ASSERT(out=="{\"k1\": \"asdwq1\"}");

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
    return 0;
}