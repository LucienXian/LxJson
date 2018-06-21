#include "json.hpp"
#include <cstdio>
#include <stdexcept> //for runtime error
#include <cstring>
#include <iostream>

namespace lxjson{

using std::string;
using std::vector;
using std::map;

static const Json json_null; // internal linkage

static void serialize(std::nullptr_t, string &out){
    out+="null";
}
static void serialize(int values, string &out){
    char c[32];
    snprintf(c, sizeof(c), "%d", values);
    out += c;
}
static void serialize(double values, string &out){
    char c[32];
    snprintf(c, sizeof(c), "%.17g", values);
    out += c;
}
static void serialize(bool values, string &out){
    out += values? "true":"false";
}
static void serialize(const string &values, string &out){
    out += "\"";
    for(auto e : values){
        switch(e){
            case '\"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(e) < 0x20) {
                    char buf[7];
                    sprintf(buf, "\\u%04X", e);
                    out += buf;
                } 
                else
                    out += e;
        }
    }
    out += "\"";
}
static void serialize(const Json::array &values, string &out){
    bool isFirst = true;
    out += "[";
    for (auto &value: values){
        if (!isFirst) out += ", ";
        value.serialize(out);
        isFirst = false;
    }
    out += "]";
}
static void serialize(const Json::object &values, string &out){
    bool isFirst = true;
    out += "{";
    for (const std::pair<string, Json> &value : values){
        if (!isFirst) 
            out += ", ";
        serialize(value.first, out);
        out += ": ";
        value.second.serialize(out);
        isFirst = false;
    }
    out += "}";
}

//JsonValue
double JsonValue::double_value() const               {throw std::runtime_error("not a number!");}
int JsonValue::int_value() const                     {throw std::runtime_error("not a number!");}
const string &JsonValue::string_value() const        {throw std::runtime_error("not a string!");}
bool JsonValue::bool_value() const                   {throw std::runtime_error("not a boolean!");}
const Json::array &JsonValue::array_value() const    {throw std::runtime_error("not a array!");}
const Json::object &JsonValue::object_value() const  {throw std::runtime_error("not a object!");}

const Json &JsonValue::operator[](size_t i) const{
    throw std::runtime_error("not an array");
}

const Json &JsonValue::operator[](const std::string key) const{
    throw std::runtime_error("not an object");
}

template<Json::JsonType tag, typename T>
class Value: public JsonValue{
protected:
    Value(const T& value):m_value(value){}
    Value(T &&value):m_value(std::move(value)){}

    Json::JsonType type() const{
        return tag;
    }

    //reinterpret_cast only guarantees that if you cast a pointer to a different type, and then reinterpret_cast it back to the original type
    bool equals(const JsonValue * other) const {
        return m_value == reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }
    bool less(const JsonValue * other) const {
        return m_value < reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }

    virtual void serialize(std::string& out) const{
        lxjson::serialize(m_value, out);
    }


    const T m_value;
};

//Type: number
class JsonInt final : public Value<Json::JsonType::T_NUMBER, int>{
public:
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonInt(double value):Value(value){}
};

class JsonDouble final : public Value<Json::JsonType::T_NUMBER, double>{
public:
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonDouble(double value):Value(value){}
};

//Type: bool
class JsonBool final : public Value<Json::JsonType::T_BOOL, bool>{
public:
    bool bool_value() const {return m_value;}

    JsonBool(bool value): Value(value){}
};

//Type string
class JsonString final : public Value<Json::JsonType::T_STRING, string>{
public:
    const string &string_value() const {return m_value;}

    JsonString(const string &value): Value(value){}
    JsonString(string &&value): Value(std::move(value)){}
};

class JsonArray final : public Value<Json::JsonType::T_ARRAY, Json::array> {
public:
    const Json::array &array_value() const {return m_value;}
    const Json &operator[](size_t i) const {
        if (i >= m_value.size()) return json_null;
        return m_value[i];
    }

    JsonArray(const Json::array &value): Value(value){}
    JsonArray(Json::array &&value): Value(std::move(value)){}
};

class JsonObject final : public Value<Json::JsonType::T_OBJECT, Json::object> {
public:
    const Json::object &object_value() const{
        return m_value;
    }
    //std::map's operator [] is not declared as const, and cannot be due to its behavior
    //in C++11, you can use the at() operator
    const Json& operator[](const string &key) const{
        return m_value.at(key);
    }

    JsonObject(const Json::object &value): Value(value){}
    JsonObject(Json::object &&value) : Value(value){}
};

class JsonNull final : public Value<Json::JsonType::T_NULL, std::nullptr_t> {
public:
    JsonNull() : Value(nullptr) {}
};

//Json constructor
static const std::shared_ptr<JsonValue> obj_null(std::make_shared<JsonNull>());
static const std::shared_ptr<JsonValue> obj_true(std::make_shared<JsonBool>(true));
static const std::shared_ptr<JsonValue> obj_false(std::make_shared<JsonBool>(false));

Json::Json() noexcept               : jv_ptr(obj_null){}
Json::Json(std::nullptr_t) noexcept : jv_ptr(obj_null){}
Json::Json(double value)            : jv_ptr(std::make_shared<JsonDouble>(value)){}
Json::Json(int value)               : jv_ptr(std::make_shared<JsonInt>(value)){} 
Json::Json(bool value)              : jv_ptr(value ? obj_true : obj_false){}
Json::Json(const std::string& value): jv_ptr(std::make_shared<JsonString>(value)){}
Json::Json(std::string&& value)     : jv_ptr(std::make_shared<JsonString>(std::move(value))) {}
Json::Json(const char* value)       : jv_ptr(std::make_shared<JsonString>(value)){}
Json::Json(const array& value)      : jv_ptr(std::make_shared<JsonArray>(value)){}
Json::Json(array&& value)           : jv_ptr(std::make_shared<JsonArray>(std::move(value))){}
Json::Json(const object & value)    : jv_ptr(std::make_shared<JsonObject>(value)){}
Json::Json(object && value)         : jv_ptr(std::make_shared<JsonObject>(std::move(value))){}


Json::Json(const Json& rhs){
    switch(rhs.type()){
        case JsonType::T_NULL:
            jv_ptr = obj_null;
            break;
        case JsonType::T_NUMBER:
            jv_ptr = std::make_shared<JsonDouble>(rhs.double_value());
            break;
        case JsonType::T_BOOL:
            jv_ptr = (rhs.bool_value()? obj_true : obj_false);
            break;
        case JsonType::T_STRING:
            jv_ptr = std::make_shared<JsonString>(rhs.string_value());
            break;
        case JsonType::T_ARRAY:
            jv_ptr = std::make_shared<JsonArray>(rhs.array_value());
            break;
        case JsonType::T_OBJECT:
            jv_ptr = std::make_shared<JsonObject>(rhs.object_value());
            break;        
    }
}

Json::Json(Json&& rhs) noexcept : jv_ptr(std::move(rhs.jv_ptr)) {}
Json::~Json() {}

Json& Json::operator=(Json rhs){
    using std::swap;
    swap(jv_ptr, rhs.jv_ptr);
}

//comparison
bool Json::operator==(const Json& rhs) const{
    if (jv_ptr->type() != rhs.type())
        return false;
    return jv_ptr->equals(rhs.jv_ptr.get());
}

bool Json::operator>(const Json& rhs) const{
    if (jv_ptr->type() != rhs.type())
        return false;
    return rhs.jv_ptr->less(jv_ptr.get());
}

//check type
Json::JsonType Json::type()   const {return jv_ptr->type();}
bool Json::is_null()    const {return type()==JsonType::T_NULL;}
bool Json::is_number()  const {return type()==JsonType::T_NUMBER;}
bool Json::is_bool()    const {return type()==JsonType::T_BOOL;}
bool Json::is_string()  const {return type()==JsonType::T_STRING;}
bool Json::is_array()   const {return type()==JsonType::T_ARRAY;}
bool Json::is_object()  const {return type()==JsonType::T_OBJECT;}

//get value
int Json::int_value()                       const {return jv_ptr->int_value();}
double Json::double_value()                 const {return jv_ptr->double_value();}
const string &Json::string_value()          const {return jv_ptr->string_value();}
bool Json::bool_value()                     const {return jv_ptr->bool_value();}
const Json::array &Json::array_value()      const {return jv_ptr->array_value();}
const Json::object &Json::object_value()    const {return jv_ptr->object_value();}

const Json& Json::operator[](size_t i) const{
    return jv_ptr->operator[](i);

}
const Json& Json::operator[](const std::string key) const{
    return jv_ptr->operator[](key);
}

void Json::serialize(std::string &out) const{
    jv_ptr->serialize(out);
}


//todo:
class jParser final {
public:
    jParser(const string& s) : start_(s.c_str()), pos_(s.c_str()){}
    Json parse(){
        switch(*start_){
            case 'n':
                return parseLiteral("null", Json(nullptr));
            case 't':
                return parseLiteral("true", Json(true));
            case 'f':
                return parseLiteral("false", Json(false));
                //break;
            case '\"':
                return parseString();
            default:
                //todo
                return json_null;
        }
    }
private:
    const char* start_;
    const char* pos_;
    void skipSpace(){
        while (*pos_ == ' ' || *pos_ == '\t' || *pos_ == '\r' || *pos_ == '\n')
            pos_++;
        start_ = pos_;
    }
    Json parseLiteral(const string &expected, Json res){
        if (strncmp(pos_, expected.c_str(), expected.size()))
            throw std::runtime_error(("Expected string ") + expected + (" but failed!"));
        pos_ += expected.size();
        return res;
    }

    string parse4hex() {
        string res;
        uint16_t ch16;
        size_t convSize;
        do{
            if (strncmp(pos_, "\\u", 2))
                throw std::runtime_error("Expected `\\uXXXX` escape sequence at position " + std::to_string(pos_-start_));
            pos_ += 2;
            if (sscanf(pos_, "%04hx", &ch16) != 1)
                throw std::runtime_error("Expected 4 hexadecimal digit sequence at position " + std::to_string((pos_ - start_)));
            convSize = utf16_to_utf8(ch16, res);
            if (convSize == static_cast<size_t>(-1))
                throw std::runtime_error("Bad utf-16 code point at position " + std::to_string(pos_ - start_));
            pos_ += 4;
        }while (!convSize);
        return res;
    }

    //reference: https://github.com/MichaelSuen-thePointer/SimpleJSON/blob/master/SimpleJSON/jparser.cpp
    size_t utf16_to_utf8(char16_t c16, std::string& s)
    {
        int nextra;

        static struct internal_state_t
        {
            unsigned long _wchar;
            unsigned short _byte, _state;
        } pst{};

        char state = static_cast<char>(pst._state); /* number of extra words expected */
        unsigned long wc = pst._wchar; /* cumulative character */

        if (state != 0)
        { /* fold in second word and convert */
            if (c16 < 0xdc00 || 0xe000 <= c16)
            {
                pst = {};
                return static_cast<size_t>(-1); /* invalid second word */
            }
            pst._state = 0;
            wc |= static_cast<unsigned long>(c16 - 0xdc00);
        }
        else if (c16 < 0xd800 || 0xdc00 <= c16)
        {
            wc = static_cast<unsigned long>(c16); /* not first word */
        }
        else
        { /* save value bits of first word for later */
            pst._state = 1;
            pst._wchar = static_cast<unsigned long>((c16 - 0xd800 + 0x0040) << 10);
            return (0);
        }

        if ((wc & ~0x7fUL) == 0)
        { /* generate a single byte */
            s += static_cast<unsigned char>(wc);
            nextra = 0;
        }
        else if ((wc & ~0x7ffUL) == 0)
        { /* generate two bytes */
            s += static_cast<unsigned char>(0xc0 | wc >> 6);
            nextra = 1;
        }
        else if ((wc & ~0xffffUL) == 0)
        { /* generate three bytes */
            s += static_cast<unsigned char>(0xe0 | wc >> 12);
            nextra = 2;
        }
        else if ((wc & ~0x1fffffUL) == 0)
        { /* generate four bytes */
            s += static_cast<unsigned char>(0xf0 | wc >> 18);
            nextra = 3;
        }
        else if ((wc & ~0x3ffffffUL) == 0)
        { /* generate five bytes */
            s += static_cast<unsigned char>(0xf8 | wc >> 24);
            nextra = 4;
        }
        else
        { /* generate six bytes */
            s += static_cast<unsigned char>(0xfc | ((wc >> 30) & 0x03));
            nextra = 5;
        }

        for (int i = nextra; i > 0; --i)
        {
            s += static_cast<unsigned char>(0x80 | ((wc >> 6 * (i - 1)) & 0x3f));
        }
        return nextra + 1;
    }


    void encode_utf8(char c, string& out){
        switch(c){
            case '\"' : case '\\': case '/':
                out.push_back(c);
                break;
            case 'b':
                out.push_back('\b');
                break;
            case 't':
                out.push_back('\t');
                break;
            case 'f':
                out.push_back('\f');
                break;
            case 'n':
                out.push_back('\n');
                break;
            case 'r':
                out.push_back('\r');
                break;
            case 'u':
                --pos_;
                out += parse4hex();
                --pos_;
                break;
            default:;
        }
    }



    Json parseString(){
        string out;
        while(true){
            char c = *(++pos_);//get the current char
            switch(c){
                case '\"':
                    start_ = ++pos_;
                    return Json(out);
                case '\0':
                    throw std::runtime_error("MISSING QUOTATION MARK!");
                case '\\':
                    encode_utf8(*++pos_, out);
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                        throw std::runtime_error("INVALID STRING CHARACTER!");
                    out.push_back(c);
            }
        }
        return Json(out);
    }


};



Json Json::parse(const string &in, string &err) noexcept{
    try {
        jParser p(in);
        return p.parse();
    } catch (std::runtime_error& e) {
        err = e.what();
        //std::cout << err << std::endl;
        return Json(nullptr);
    }

}


}
