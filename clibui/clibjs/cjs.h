﻿//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJS_H
#define CLIBJS_CJS_H

#include <string>
#include "cjsruntime.h"

namespace clib {

    class cjs {
    public:
        cjs();
        ~cjs();

        cjs(const cjs&) = delete;
        cjs& operator=(const cjs&) = delete;

        int exec(const std::string& filename, const std::string& input);

        void resize();
        void reset_ips();
        void hit(int n);
        bool try_input(int c, bool ch = true);
        int cursor() const;
        void clear_cache();

        bool run(int cycle, int& cycles);
        int get_state() const;
        void set_state(int);

        int get_frame() const;
        void clear_frame();

        void change_target();

    private:
        void init_lib();

    private:
        cjsruntime rt;
    };
}

#endif //CLIBJS_CJS_H
