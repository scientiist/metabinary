#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <netinet/in.h>
#include <cstring>


/* Metabinary is envisioned as a standard protocol for;
 * high performance networking, as well as a versatile storage format for data;
 * encoding both large blocks and small metadata tokens in a single binary file.
 *
 */
namespace metabinary {
    static int write_uint8(uint8_t* buf, int index, uint8_t val)      {
        int offset = index;
        buf[offset] = htons(val);
        offset++;
        return offset-index;
    }
    static int write_uint16(uint8_t* buf, int index, uint16_t val)    {
        int offset = index;
        uint16_t data = htons(val);
        memcpy(buf+offset, &data, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        return offset - index;
    }
    static int write_uint32(uint8_t* buf, int index, uint32_t val)    {
        int offset = index;
        uint32_t data = htonl(val);
        memcpy(buf+offset, &data, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        return offset - index;
    }
    static int write_uint64(uint8_t* buf, int index, uint64_t val)    {
        int offset = index;
        uint64_t data = htonl(val);
        memcpy(buf+offset, &data, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        return offset - index;
    }
    static int write_int8(uint8_t* buf, int index, int8_t val)         {
        int offset = index;
        int8_t data = htons(val);
        memcpy(buf+offset, &data, sizeof(int8_t));
        offset += sizeof(int8_t);
        return offset - index;
    }
    static int write_int16(uint8_t* buf, int index, int16_t val)       {
        int offset = index;
        int16_t data = htons(val);
        memcpy(buf+offset, &data, sizeof(int16_t));
        offset += sizeof(int16_t);
        return offset - index;
    }
    static int write_int32(uint8_t* buf, int index, int32_t val)       {
        int offset = index;
        int32_t data = htonl(val);
        memcpy(buf+offset, &data, sizeof(int32_t));
        offset += sizeof(int32_t);
        return offset - index;
    }
    static int write_int64(uint8_t* buf, int index, int64_t val)       {
        int offset = index;
        int64_t data = htonl(val);
        memcpy(buf+offset, &data, sizeof(int64_t));
        offset += sizeof(int64_t);
        return offset - index;
    }
    static int write_float(uint8_t* buf, int index, float val)        {
        int offset = index;
        memcpy(buf+offset, &val, sizeof(val));
        offset += sizeof(float);
        return offset - index;
    }
    static int write_double(uint8_t* buf, int index, double val)      {
        int offset = index;
        memcpy(buf+offset, &val, sizeof(val));
        offset += sizeof(double);
        return offset - index;
    }
    static int write_string(uint8_t* buf, int index, std::string val) {
        int offset = index;
        // Add Payload Length
        offset += write_uint32(buf, offset, htonl(val.length()));
        // Add Payload (As UTF8-Encoded String)
        memcpy(buf+offset, &val, val.length());
        offset += val.length();
        return offset - index;
    }
    static uint8_t read_uint8(uint8_t* buf, int index)   {
        uint8_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(uint8_t));
        return outpt;
    }
    static uint16_t read_uint16(uint8_t* buf, int index) {
        uint16_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(uint16_t));
        return ntohs(outpt);
    }
    static uint32_t read_uint32(uint8_t* buf, int index) {
        uint32_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(uint32_t));
        return ntohl(outpt);
    }
    static uint64_t read_uint64(uint8_t* buf, int index) {
        uint64_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(uint64_t));
        return ntohl(outpt);
    }
    static int8_t read_int8(uint8_t* buf, int index)      {
        int8_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(int8_t));
        return ntohs(outpt);
    }
    static int16_t read_int16(uint8_t* buf, int index)    {
        int16_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(int16_t));
        return ntohs(outpt);
    }
    static int32_t read_int32(uint8_t* buf, int index)    {
        int32_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(int32_t));
        return ntohl(outpt);
    }
    static int64_t read_int64(uint8_t* buf, int index)    {
        int64_t outpt = 0;
        memcpy(&outpt, buf+index, sizeof(int64_t));
        return ntohl(outpt);
    }
    static float read_float(uint8_t* buf, int index)      {
        float outpt = 0;
        memcpy(&outpt, buf+index, sizeof(float));
        return outpt;
    }
    static double read_double(uint8_t* buf, int index)    {
        double outpt = 0;
        memcpy(&outpt, buf+index, sizeof(double));
        return outpt;
    }
    static std::string read_string(uint8_t* buf, int index) {
        uint32_t str_len = read_uint32(buf, index);
        char val[str_len];
        memcpy(&val, buf+index, str_len);
        std::string out(val, str_len);
        return out;
    }
    typedef enum {
        tag_end = 0,
        tag_uint8, tag_uint16, tag_uint32, tag_uint64,
        tag_sint8, tag_sint16, tag_sint32, tag_sint64,
        tag_float, tag_double,
        tag_byte_array,
        tag_string,
        tag_list,
        tag_compound,
        tag_identifier = -1,
        tag_primitive = -2,
    } tag_type_t;
    class tag {
    public:
        std::string name;

        tag() {}
        tag(std::string name) {}

        virtual int serialize(uint8_t* buf, int startidx)
        {
            return 0;
        }

        //static tag deserialize(byte* data);
        // Encodes name length + utf8 string
        int write_name(uint8_t* buf, int startidx)
        {
            return write_string(buf, startidx, this->name);
        }

        static int write_type(uint8_t* buf, int startidx, tag_type_t tag_type)
        {
            int index = startidx;
            buf[index] = tag_type;
            index++;
            return index-startidx;
        }
        virtual int write_payload(uint8_t* buf, int startidx) {}
    private:

    protected:
    };
    class end_tag : public tag {
    public:
        int serialize(uint8_t *buf, int startidx) override {}
    };
    class uint8_tag : public tag {
    private:
        uint8_t payload;
    public:
        uint8_tag() {}
        uint8_tag(std::string name) {}
        uint8_tag(std::string name, uint8_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_uint8);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override
        {
            return write_uint8(buf, startidx, payload);
        }
    };
    class uint16_tag : public tag {
    private:
        uint16_t payload;
    public:
        uint16_tag(std::string name, uint16_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_uint16);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override
        {
            return write_uint16(buf, startidx, payload);
        }
    };
    class uint32_tag : public tag {
    private:
        uint32_t payload;
    public:
        uint32_tag() {}
        uint32_tag(std::string name) {}
        uint32_tag(std::string name, uint32_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override
        {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_uint32);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override
        {
            return write_uint32(buf, startidx, payload);
        }
    };
    class uint64_tag : public tag {
    public:
        uint64_tag() {}
        uint64_tag(std::string name) {}
        uint64_tag(std::string name, uint64_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override
        {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_uint64);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_uint64(buf, startidx, payload);
        }
    private:
        uint64_t payload;
    };
    class sint8_tag : public tag {
        int8_t payload;
    public:
        sint8_tag() {}
        sint8_tag(std::string name) {}
        sint8_tag(std::string name, int8_t data)
        {
            this->name = name;
            this->payload = data;
        }
        int serialize(uint8_t *buf, int startidx) {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_sint8);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset - startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_int8(buf, startidx, payload);
        }
    };
    class sint16_tag : public tag {
        int16_t payload;
    public:
        sint16_tag() { }
        sint16_tag(std::string name) { }
        sint16_tag(std::string name, int16_t data) {
            this->name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx)
        {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_sint16);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_int16(buf, startidx, payload);
        }
    };
    class sint32_tag : public tag {
        int32_t payload;
    public:
        sint32_tag() {}
        sint32_tag(std::string name) {}
        sint32_tag(std::string name, int32_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx)
        {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_sint32);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset - startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_int32(buf, startidx, payload);
        }
    };
    class sint64_tag : public tag {
        int64_t payload;
    public:
        sint64_tag() {}
        sint64_tag(std::string name) {}
        sint64_tag(std::string name, int64_t data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_sint64);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset - startidx;
        }
        int write_payload(uint8_t *buf, int index) override {
            return write_int64(buf, index, payload);
        }
    };
    class float_tag : public tag {
        float payload;
    public:
        float_tag() {}
        float_tag(std::string name) {}
        float_tag(std::string name, float data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override {
            int offset = startidx;
            offset+=write_type(buf, offset, metabinary::tag_float);
            offset+=write_name(buf, offset);
            offset+=write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_float(buf, startidx, payload);
        }
    };
    class double_tag : public tag {
        double payload;
    public:
        double_tag() {}
        double_tag(std::string name) {}
        double_tag(std::string name, double data)
        {
            this->name = name;
            payload = data;
        }
        int serialize(uint8_t *buf, int startidx) override {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_double);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_double(buf, startidx, payload);
        }
    };
    class string_tag : public tag {
        std::string payload;
    public:
        int serialize(uint8_t *buf, int startidx) override
        {
            int offset = startidx;
            offset += write_type(buf, offset, metabinary::tag_string);
            offset += write_name(buf, offset);
            offset += write_payload(buf, offset);
            return offset-startidx;
        }
        int write_payload(uint8_t *buf, int startidx) override {
            return write_string(buf, startidx, payload);
        }

    };
    template <typename T> class list_tag : public tag {
    public:
    private:
    };
    class compound_tag : public tag {
    private:
        std::vector<tag*> payload;
    public:
        compound_tag() {}
        compound_tag(std::string name) {}
        compound_tag(std::string name, std::vector<tag*> tags)
        {
            this->name = name;
            payload = tags;
        }
        tag* get(std::string name) const {

            for (auto& child : payload)
            {
                if (child->name == name)
                    return child;
            }
        }
        int serialize(uint8_t* buffer, int startidx)
        {
            int offset = startidx;

            offset += write_type(buffer, offset, metabinary::tag_compound);
            offset += write_name(buffer, offset);

            // Add Serialized Child Tags
            for (auto& tag : payload)
                offset += tag->serialize(buffer, offset);

            // Add END Tag
            buffer[offset] = tag_end;
            offset++;

            // Return used space
            return offset-startidx;
        }
        void add_byte() {}
        void add_short() {}
        void add_int() {}
        void add_long() {}
        void add_float() {}
        void add_double() {}
    };
    class root_tag : public compound_tag {
    public:
        root_tag() : compound_tag() {};
        root_tag(std::string name) : compound_tag(name) {}
        root_tag(std::string name, std::vector<tag*> tags) : compound_tag(name,  tags) {}
    };
    struct file { root_tag tag; };
    static root_tag deserialize(uint8_t* buf)
    {

    }
}

int main() {
    using namespace metabinary;

    auto tone = new uint16_tag{"b", 123};

    compound_tag item_meta = {"", {
        new uint16_tag{"id", 64},
        new uint16_tag{"quantity", 64},
        tone
    }};

    root_tag demo_file { "DEMO METABINARY FILE", {
        new uint8_tag{"BITTY", 255},
        new uint16_tag{"SHORTY", 420},
        new uint32_tag{"INTEGER", 41221},
        new uint64_tag{"BIGINT", 99999999999999},

        new compound_tag{"STATE", {
            new float_tag{"x", 0.25f},
            new float_tag{"y", 0.25f},
            new double_tag{"magic_number", 3.1414951},
        }},
    }};

    tag *t = demo_file.get("SHORTY");
    std::cout << t->name << std::endl;
    uint8_t  byte_buff[9999];
    demo_file.serialize(byte_buff, 0);
    // copy byte to char buff for writing
    char output[sizeof(byte_buff)];

    memcpy(output, &byte_buff, sizeof(byte_buff));
    std::ofstream output_buff("test.bin", std::ios::out|std::ios::binary);
    output_buff.write(output, sizeof(output));
    output_buff.close();
    return 0;
}