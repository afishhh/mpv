/*
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_utils.h"

#include <limits.h>

#include "misc/codepoint_width.h"

#define W(s) term_disp_width((bstr)bstr0_lit(s), INT_MAX, &(const unsigned char *){NULL})

int main(void) {
    assert_int_equal(W("A"), 1);               // Single ASCII character
    assert_int_equal(W("ABC"), 3);             // Multiple ASCII characters

    assert_int_equal(W("\u3042"), 2);          // Full-width Japanese Hiragana 'あ' (U+3042)
    assert_int_equal(W("\u4F60"), 2);          // Full-width Chinese character '你' (U+4F60)
    assert_int_equal(W("\u4F60\u597D"), 4);    // Two full-width Chinese characters '你好' (U+4F60 U+597D)

    assert_int_equal(W("a\u0301"), 1);         // 'a' + combining acute accent '́' (U+0301)
    assert_int_equal(W("e\u0301"), 1);         // 'e' + combining acute accent '́' (U+0301)
    assert_int_equal(W("a\u0308"), 1);         // 'a' + combining diaeresis '̈' (U+0308)

    // Family emoji: 👩‍❤️‍👩 (Woman + ZWJ + Heart + ZWJ + Woman;
    // Code points: U+1F469 U+200D U+2764 U+FE0F U+200D U+1F469)
    assert_int_equal(W("\U0001F469\u200D\u2764\uFE0F\u200D\U0001F469"), 2);
    // Man emoji with skin tone modifier
    assert_int_equal(W("\U0001F468\U0001F3FE"), 2);
    // Person Shrugging + skin tone modifier + ZWJ + woman sign + variant selector
    assert_int_equal(W("\U0001F937\U0001F3FE\u200D\U00002640\U0000FE0F"), 2);
    // Regional indicator symbols forming the flag of Poland
    assert_int_equal(W("\U0001F1F5\U0001F1F1"), 2);

    assert_int_equal(W("\n"), 0);              // Newline (should not take up any visual space)
    assert_int_equal(W("\t"), 8);              // Tab (tabstop assumed to be 8)
    assert_int_equal(W("\0"), 0);              // Null character (non-visible)

    assert_int_equal(W("A\u3042"), 3);         // ASCII 'A' + full-width Japanese Hiragana 'あ' (U+3042)
    assert_int_equal(W("a\u0301A"), 2);        // Combining character 'á' (a + U+0301) and ASCII 'A'

    // Grapheme cluster + ASCII 'A' + full-width Japanese Hiragana 'あ' (U+3042)
    assert_int_equal(W("\U0001F469\u200D\u2764\uFE0F\u200D\U0001F469A\u3042"), 5);

    assert_int_equal(W("A\nB"), 2);            // ASCII characters with newline (newline should not affect width)
    assert_int_equal(W("ABC\tDEF"), 11);       // Tab inside a string

    // Single illegal character
    assert_int_equal(W("abc" "\xFF" "def"), 7);
    // Codepoint sequence cut off by one byte
    assert_int_equal(W("abc"  "\xF0\x9F\x98" "def"), 7);

    // Examples from https://www.unicode.org/versions/Unicode17.0.0/core-spec/chapter-3/#G66453
    // Table 3-8. U+FFFD for Non-Shortest Form Sequences
    assert_int_equal(W("\xC0\xAF\xE0\x80\xBF\xF0\x81\x82\x41"), 9);
    // Table 3-9. U+FFFD for Ill-Formed Sequences for Surrogates
    assert_int_equal(W("\xED\xA0\x80\xED\xBF\xBF\xED\xAF\x41"), 9);
    // Table 3-10. U+FFFD for Other Ill-Formed Sequences
    assert_int_equal(W("\xF4\x91\x92\x93\xFF\x41\x80\xBF\x42"), 9);
    // Table 3-11. U+FFFD for Truncated Sequences
    assert_int_equal(W("\xE1\x80\xE2\xF0\x91\x92\xF1\xBF\x41"), 5);

    // ASCII characters with color
    assert_int_equal(W("\033[31mABC\033[0m\033[32mDEF\033[0m"), 6);

    // ASCII characters with color and a newline
    assert_int_equal(W("\033[31mABC\033[0m\033[32mDEF\033[0m\n"), 6);

    // ASCII characters with carriage return
    assert_int_equal(W("ABC\rDEF"), 3);



    // Two-byte sequence (C0 80)
    assert_int_equal(W("\xc0\x80"), 2);
    // Three-byte sequence (E0 80 80)
    assert_int_equal(W("\xe0\x80\x80"), 3);
    // Four-byte sequence (F0 80 80 80)
    assert_int_equal(W("\xf0\x80\x80\x80"), 4);
    // Five-byte sequence (F8 80 80 80 80)
    assert_int_equal(W("\xf8\x80\x80\x80\x80"), 5);
    // Six-byte sequence (FC 80 80 80 80 80)
    assert_int_equal(W("\xfc\x80\x80\x80\x80\x80"), 6);
    // Two-byte sequence (C1 BF)
    assert_int_equal(W("\xc1\xbf"), 2);
    // Three-byte sequence (E0 81 BF)
    assert_int_equal(W("\xe0\x81\xbf"), 3);
    // Four-byte sequence (F0 80 81 BF)
    assert_int_equal(W("\xf0\x80\x81\xbf"), 4);
    // Five-byte sequence (F8 80 80 81 BF)
    assert_int_equal(W("\xf8\x80\x80\x81\xbf"), 5);
    // Six-byte sequence (FC 80 80 80 81 BF)
    assert_int_equal(W("\xfc\x80\x80\x80\x81\xbf"), 6);
    // Three-byte sequence (E0 82 80)
    assert_int_equal(W("\xe0\x82\x80"), 3);
    // Four-byte sequence (F0 80 82 80)
    assert_int_equal(W("\xf0\x80\x82\x80"), 4);
    // Five-byte sequence (F8 80 80 82 80)
    assert_int_equal(W("\xf8\x80\x80\x82\x80"), 5);
    // Six-byte sequence (FC 80 80 80 82 80)
    assert_int_equal(W("\xfc\x80\x80\x80\x82\x80"), 6);
    // Three-byte sequence (E0 9F BF)
    assert_int_equal(W("\xe0\x9f\xbf"), 3);
    // Four-byte sequence (F0 80 9F BF)
    assert_int_equal(W("\xf0\x80\x9f\xbf"), 4);
    // Five-byte sequence (F8 80 80 9F BF)
    assert_int_equal(W("\xf8\x80\x80\x9f\xbf"), 5);
    // Six-byte sequence (FC 80 80 80 9F BF)
    assert_int_equal(W("\xfc\x80\x80\x80\x9f\xbf"), 6);
    // Four-byte sequence (F0 80 A0 80)
    assert_int_equal(W("\xf0\x80\xa0\x80"), 4);
    // Five-byte sequence (F8 80 80 A0 80)
    assert_int_equal(W("\xf8\x80\x80\xa0\x80"), 5);
    // Six-byte sequence (FC 80 80 80 A0 80)
    assert_int_equal(W("\xfc\x80\x80\x80\xa0\x80"), 6);
    // Four-byte sequence (F0 8F BF BF)
    assert_int_equal(W("\xf0\x8d\xbf\xbf"), 4);
    // Five-byte sequence (F8 80 8F BF BF)
    assert_int_equal(W("\xf8\x80\x8f\xbf\xbf"), 5);
    // Six-byte sequence (FC 80 80 8F BF BF)
    assert_int_equal(W("\xfc\x80\x80\x8f\xbf\xbf"), 6);
    // Five-byte sequence (F8 80 90 80 80)
    assert_int_equal(W("\xf8\x80\x90\x80\x80"), 5);
    // Six-byte sequence (FC 80 80 90 80 80)
    assert_int_equal(W("\xfc\x80\x80\x90\x80\x80"), 6);
    // Five-byte sequence (F8 84 8F BF BF)
    assert_int_equal(W("\xf8\x84\x8f\xbf\xbf"), 5);
    // Six-byte sequence (FC 80 84 8F BF BF)
    assert_int_equal(W("\xfc\x80\x84\x8f\xbf\xbf"), 6);
    // One past Unicode (F4 90 80 80)
    assert_int_equal(W("\xf4\x90\x80\x80"), 4);
    // Longest five-byte sequence (FB BF BF BF BF)
    assert_int_equal(W("\xfb\xbf\xbf\xbf\xbf"), 5);
    // Longest six-byte sequence (FD BF BF BF BF BF)
    assert_int_equal(W("\xfd\xbf\xbf\xbf\xbf\xbf"), 6);
    // First surrogate (ED A0 80)
    assert_int_equal(W("\xed\xa0\x80"), 3);
    // Last surrogate (ED BF BF)
    assert_int_equal(W("\xed\xbf\xbf"), 3);
    // CESU-8 surrogate pair (ED A0 BD ED B2 A9)
    assert_int_equal(W("\xed\xa0\xbd\xed\xb2\xa9"), 6);
    // One past Unicode as five-byte sequence (F8 84 90 80 80)
    assert_int_equal(W("\xf8\x84\x90\x80\x80"), 5);
    // One past Unicode as six-byte sequence (FC 80 84 90 80 80)
    assert_int_equal(W("\xfc\x80\x84\x90\x80\x80"), 6);
    // First surrogate as four-byte sequence (F0 8D A0 80)
    assert_int_equal(W("\xf0\x8d\xa0\x80"), 4);
    // Last surrogate as four-byte sequence (F0 8D BF BF)
    assert_int_equal(W("\xf0\x8d\xbf\xbf"), 4);
    // CESU-8 surrogate pair as two four-byte overlongs (F0 8D A0 BD F0 8D B2 A9)
    assert_int_equal(W("\xf0\x8d\xa0\xbd\xf0\x8d\xb2\xa9"), 8);
    // One (80)
    assert_int_equal(W("\x80"), 1);
    // Two (80 80)
    assert_int_equal(W("\x80\x80"), 2);
    // Three (80 80 80)
    assert_int_equal(W("\x80\x80\x80"), 3);
    // Four (80 80 80 80)
    assert_int_equal(W("\x80\x80\x80\x80"), 4);
    // Five (80 80 80 80 80)
    assert_int_equal(W("\x80\x80\x80\x80\x80"), 5);
    // Six (80 80 80 80 80 80)
    assert_int_equal(W("\x80\x80\x80\x80\x80\x80"), 6);
    // Seven (80 80 80 80 80 80 80)
    assert_int_equal(W("\x80\x80\x80\x80\x80\x80\x80"), 7);
    // After valid two-byte (C2 B6 80)
    assert_int_equal(W("\xc2\xb6\x80"), 2);
    // After valid three-byte (E2 98 83 80)
    assert_int_equal(W("\xe2\x98\x83\x80"), 2);
    // After valid four-byte (F0 9F 92 A9 80)
    assert_int_equal(W("\xf0\x9f\x92\xa9\x80"), 3);
    // After five-byte (FB BF BF BF BF 80)
    assert_int_equal(W("\xfb\xbf\xbf\xbf\xbf\x80"), 6);
    // After six-byte (FD BF BF BF BF BF 80)
    assert_int_equal(W("\xfd\xbf\xbf\xbf\xbf\xbf\x80"), 7);
    // Two-byte lead (C2)
    assert_int_equal(W("\xc2"), 1);
    // Three-byte lead (E2)
    assert_int_equal(W("\xe2"), 1);
    // Three-byte lead and one trail (E2 98)
    assert_int_equal(W("\xe2\x98"), 1);
    // Four-byte lead (F0)
    assert_int_equal(W("\xf0"), 1);
    // Four-byte lead and one trail (F0 9F)
    assert_int_equal(W("\xf0\x9f"), 1);
    // Four-byte lead and two trails (F0 9F 92)
    assert_int_equal(W("\xf0\x9f\x92"), 1);
    // FE (FE)
    assert_int_equal(W("\xfe"), 1);
    // FE and trail (FE 80)
    assert_int_equal(W("\xfe\x80"), 2);
    // FF (FF)
    assert_int_equal(W("\xff"), 1);
    // FF and trail (FF 80)
    assert_int_equal(W("\xff\x80"), 2);

    bstr str = bstr0("ABCDEF");
    const unsigned char *cut_pos;

    cut_pos = NULL;
    assert_int_equal(term_disp_width(str, 3, &cut_pos), 3);
    assert_int_equal(cut_pos - str.start, 3);

    cut_pos = NULL;
    assert_int_equal(term_disp_width(str, -2, &cut_pos), 0);
    assert_int_equal(cut_pos - str.start, 0);

    cut_pos = NULL;
    assert_int_equal(term_disp_width(str, str.len, &cut_pos), 6);
    if (cut_pos) {
        printf("%s:%d: cut_pos != NULL\n", __FILE__, __LINE__);
        fflush(stdout);
        abort();
    }
}
