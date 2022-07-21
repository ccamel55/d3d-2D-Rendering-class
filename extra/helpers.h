#pragma once

#include <Windows.h>

#include <iostream>
#include <exception>

namespace RENDER {
    
    using RENDER_MATRIX = float[4][4];
        
    inline UINT MAX_VERTICES =  1024 * 4 * 3;

    enum E_FONT_FLAGS : UINT {
        FONT_CENTERED_X = 0x1,
        FONT_CENTERED_Y = 0x2,
        FONT_CENTERED = FONT_CENTERED_X | FONT_CENTERED_Y,
        FONT_RIGHT_ALIGNED = 0x4,
        FONT_OUTLINE = 0x8,
    };

    namespace HELPER {

        // credits Microsoft DirectXTK
        class com_exception : public std::exception {
        public:
            com_exception(HRESULT hr) : result(hr) {}

            const char* what() const override {

                static char s_str[64] = {};
                sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));

                return s_str;
            }

        private:
            HRESULT result;
        }; 

        inline void throwIfFailed(HRESULT hr) {

            if (FAILED(hr))
                throw com_exception(hr);
        }

        template <class t>
        inline void safeRelease(t* ptr) {

            if (ptr){

                ptr->Release();
                ptr = nullptr;
            }
        }
    }
} 