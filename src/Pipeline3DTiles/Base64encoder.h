#ifndef __BASE64ENCODER_H_
#define __BASE64ENCODER_H_
#include <iostream>
#include <sstream>
#include <vector>
const int BUFFERSIZE = 8192;

typedef enum
{
    step_a, step_b, step_c, step_d
} base64_decodestep;

typedef enum
{
    step_A, step_B, step_C
} base64_encodestep;

typedef struct
{
    base64_encodestep step;
    char result;
    int stepcount;
} base64_encodestate;

typedef struct
{
    base64_decodestep step;
    char plainchar;
} base64_decodestate;

class Base64encoder
{
public:
    Base64encoder(int buffersize_in = BUFFERSIZE)
        : _buffersize(buffersize_in)
    {}

    int encode(char value_in);

    int encode(const char* code_in, const int length_in, char* plaintext_out);

    int encode_end(char* plaintext_out);

    void encode(std::istream& istream_in, std::ostream& ostream_in);

    void encode(const char* chars_in, int length_in, std::string& code_out);

private:
    base64_encodestate _state;
    int _buffersize;
};
class Base64decoder
{
public:
    Base64decoder(int buffersize_in = BUFFERSIZE)
        : _buffersize(buffersize_in)
    {}

    int decode(char value_in);

    int decode(const char* code_in, const int length_in, char* plaintext_out);

    void decode(std::istream& istream_in, std::ostream& ostream_in);

    // Decode strings, returns one char* of appropriate size
    // Note that deallocation of char* is up to the caller of this method
    char* decode(const std::vector<std::string>& str_in, std::vector<unsigned int>& pos_out);

private:
    base64_decodestate _state;
    int _buffersize;
};
#endif
