/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE. MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
#pragma once
#include <string>

namespace MyLibrary
{

enum Color
{
        red,
        green,
        blue
};

/**
 * @brief Example class to demonstrate the features of the custom CSS.
 *
 * @author jothepro
 *
 */
class Example
{
public:
        /**
         * @brief brief summary
         *
         * doxygen test documentation
         *
         * @param test this is the only parameter of this test function. It does
         * nothing!
         *
         * # Supported elements
         *
         * These elements have been tested with the custom CSS.
         *
         * ## Tables
         *
         * The table content is scrollable if the table gets too wide.
         *
         * | first_column | second_column | third_column | fourth_column |
         * fifth_column | sixth_column | seventh_column | eighth_column |
         * ninth_column |
         * |--------------|---------------|--------------|---------------|--------------|--------------|----------------|---------------|--------------|
         * | 1            | 2             | 3            | 4             | 5 | 6
         * | 7 | 8 | 9            |
         *
         *
         * ## Lists
         *
         * - element 1
         * - element 2
         *
         * 1. element 1
         *    ```
         *    code in lists
         *    ```
         * 2. element 2
         *
         * ## Quotes
         *
         * > Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do
         * eiusmod tempor incididunt > ut labore et dolore magna aliqua. Vitae
         * proin sagittis nisl rhoncus mattis rhoncus urna neque viverra. >
         * Velit sed ullamcorper morbi tincidunt ornare.
         * >
         * > Lorem ipsum dolor sit amet consectetur adipiscing elit duis.
         * *- jothepro*
         *
         * ## Code block
         *
         * ```cpp
         * auto x = "code within md fences (```)";
         * ```
         *
         * @code{.cpp}
         * // code within @code block
         * while(true) {
         *    auto example = std::make_shared<Example>(5);
         *    example->test("test");
         * }
         *
         * @endcode
         *
         *     // code within indented code block
         *     auto test = std::shared_ptr<Example(5);
         *
         *
         * Inline `code` elements in a text. *Lorem ipsum dolor sit amet,
         * consectetur adipiscing elit, sed do eiusmod tempor incididunt ut
         * labore et dolore magna aliqua.* This also works within multiline text
         * and does not break the `layout`.
         *
         *
         * ## Special hints
         *
         * @warning this is a warning only for demonstration purposes
         *
         * @note this is a note to show that notes work. They can also include
         * `code`:
         * @code{.c}
         * void this_looks_awesome();
         * @endcode
         *
         * @bug example bug
         *
         * @deprecated None of this will be deprecated, because it's beautiful!
         *
         * @invariant This is an invariant
         *
         * @pre This is a precondition
         *
         * @todo This theme is never finished!
         *
         * @remark This is awesome!
         *
         */
        std::string test( const std::string& test );

        virtual int virtualfunc() = 0;

        static bool staticfunc();
};

}  // namespace MyLibrary
