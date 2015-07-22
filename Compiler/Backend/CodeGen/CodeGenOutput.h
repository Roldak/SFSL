//
//  CodeGenOutput.h
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CodeGenOutput__
#define __SFSL__CodeGenOutput__

#include <iostream>
#include <vector>
#include <list>
#include "../../../Utils/Utils.h"
#include "../../../Compiler/Common/CompilationContext.h"

namespace sfsl {

namespace out {

    /**
     * @brief Interface for objects storing informations
     * relative to a location in the output code
     */
    class Cursor : public common::MemoryManageable {
    protected:
        Cursor();
        virtual ~Cursor();
    };

    template<typename T>
    /**
     * @brief Interface representing the destination of the generated code
     */
    class CodeGenOutput {
    public:

        virtual ~CodeGenOutput() {}

        /**
         * @brief Appends a value to the destination object
         * @param t The value to be added
         * @return This
         */
        virtual CodeGenOutput& operator <<(const T& t) = 0;

        /**
         * @return A cursor to the actual location
         */
        virtual Cursor* here() const = 0;

        /**
         * @return A cursor to the end of the output object
         */
        virtual Cursor* end() const = 0;

        /**
         * @brief Sets the output position for the next instructions
         * to the one pointed by the cursor
         * @param c The cursor to follow
         */
        virtual void seek(Cursor* c) = 0;
    };

    template<typename T>
    /**
     * @brief Implementation of the CodeGenOutput interface writing the emitted code into a vector
     */
    class LinkedListOutput : public CodeGenOutput<T> {
    protected:

        struct Node final : public common::MemoryManageable {
            Node(Node* n, const T& val) : next(n), value(val) {}
            virtual ~Node() {}

            Node* next;
            T value;
        };

        class LinkedListOutputCursor : public Cursor {
        public:

            LinkedListOutputCursor(Node* node) : pos(node) {}
            virtual ~LinkedListOutputCursor() {}

            Node* pos;
        };

    public:

        LinkedListOutput(CompCtx_Ptr& ctx) : _ctx(ctx), _here(nullptr), _end(nullptr) {}

        virtual ~LinkedListOutput() {}

        virtual CodeGenOutput<T>& operator <<(const T& t) override {
            if (_here) {
                _here->next = _ctx->memoryManager().New<Node>(_here->next, t);
                if (_end == _here) {
                    _end = _end->next;
                }
                _here = _here->next;
            } else {
                _here = _ctx->memoryManager().New<Node>(nullptr, t);
                _here->next = _here;
                _end = _here;
            }
            return *this;
        }

        virtual Cursor* here() const override {
            return _ctx->memoryManager().New<LinkedListOutputCursor>(_here);
        }

        virtual Cursor* end() const override {
            return _ctx->memoryManager().New<LinkedListOutputCursor>(_end);
        }

        virtual void seek(Cursor* c) override {
            if (LinkedListOutputCursor* voc = dynamic_cast<LinkedListOutputCursor*>(c)) {
                _here = voc->pos;
            } else {
                throw common::CompilationFatalError("Failed to seek to cursor: Wrong cursor type");
            }
        }

        std::vector<T> toVector() const {
            std::vector<T> toRet;
            if (_here) {
                for (Node* cur = _end->next; cur != _end; cur = cur->next) {
                    toRet.push_back(cur->value);
                }
                toRet.push_back(_end->value);
            }
            return toRet;
        }

    protected:

        CompCtx_Ptr& _ctx;

        Node* _here;
        Node* _end;
    };
}

}

#endif
