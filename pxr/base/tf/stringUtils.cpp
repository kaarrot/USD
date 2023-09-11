//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#include "pxr/pxr.h"

#include "pxr/base/tf/stringUtils.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/pathUtils.h"
#include "pxr/base/tf/tf.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/arch/inttypes.h"
#include "pxr/base/arch/math.h"
#include "pxr/base/arch/vsnprintf.h"

#include <algorithm>
#include <climits>
#include <cstdarg>
#include <ctype.h>
#include <limits>
#include <string>
#include <utility>
#include <tuple>
#include <type_traits>
#include <vector>
#include <memory>

#include "pxrDoubleConversion/double-conversion.h"
#include "pxrDoubleConversion/utils.h"

#if defined(ARCH_OS_WINDOWS)
#include <Shlwapi.h>
#endif

using std::list;
using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::vector;

PXR_NAMESPACE_OPEN_SCOPE

namespace {
    bool _IsASCIIValue(char c)
    {
        return (c & (1<<7)) == 0;
    }
}

string
TfVStringPrintf(const std::string& fmt, va_list ap)
{
    return ArchVStringPrintf(fmt.c_str(), ap);
}

string
TfVStringPrintf(const char *fmt, va_list ap)
{
    return ArchVStringPrintf(fmt, ap);
}

string
TfStringPrintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string s = ArchVStringPrintf(fmt, ap);
    va_end(ap);
    return s;
}

static inline double
_StringToDoubleImpl(const char *ptr, int len)
{
    pxr_double_conversion::StringToDoubleConverter
        strToDouble(pxr_double_conversion::DoubleToStringConverter::NO_FLAGS,
                    /* empty_string_value */ 0,
                    /* junk_string_value */ 0,
                    /* infinity symbol */ "inf",
                    /* nan symbol */ "nan");
    int numDigits_unused;
    return strToDouble.StringToDouble(ptr, len, &numDigits_unused);
}

double
TfStringToDouble(const char *ptr, int len)
{
    return _StringToDoubleImpl(ptr, len);
}    

double
TfStringToDouble(const char *ptr)
{
    return _StringToDoubleImpl(ptr, static_cast<int>(strlen(ptr)));
}

double
TfStringToDouble(const string& s)
{
    return TfStringToDouble(s.c_str());
}

// Convert a sequence of digits in a string to a negative integral value of
// signed integral type Int.  Caller is responsible for ensuring that p points
// to a valid sequence of digits.  The minus sign '-' may not appear.
//
// If the resulting value would be less than the minimum representable value,
// return that minimum representable value and set *outOfRange to true (if
// outOfRange is not NULL).
template <class Int>
static std::enable_if_t<std::is_signed<Int>::value, Int>
_StringToNegative(const char *p, bool *outOfRange)
{
    const Int M = std::numeric_limits<Int>::min();
    Int result = 0;
    while (*p >= '0' && *p <= '9') {
        Int digit = (*p++ - '0');
        // If the new digit would exceed the range, bail.  The expression below
        // is equivalent to 'result < (M + digit) / 10', but it avoids division.
        if (ARCH_UNLIKELY(result < ((M / 10) + (-digit < (M % 10))))) {
            if (outOfRange)
                *outOfRange = true;
            return M;
        }
        result = result * 10 - digit;
    }
    return result;
}

// Convert a sequence of digits in a string to a positive integral value of
// integral type Int.  Caller is responsible for ensuring that p points to a
// valid sequence of digits.
//
// If the resulting value would be greater than the maximum representable value,
// return that maximum representable value and set *outOfRange to true (if
// outOfRange is not NULL).
template <class Int>
static Int
_StringToPositive(const char *p, bool *outOfRange)
{
    const Int R = 10;
    const Int M = std::numeric_limits<Int>::max();
    Int result = 0;
    while (*p >= '0' && *p <= '9') {
        Int digit = (*p++ - '0');
        // If the new digit would exceed the range, bail.  The expression below
        // is equivalent to 'result > (M - digit) / 10', but it avoids division.
        if (ARCH_UNLIKELY(result > ((M / R) - (digit > (M % R))))) {
            if (outOfRange)
                *outOfRange = true;
            return M;
        }
        result = result * 10 + digit;
    }
    return result;
}

long
TfStringToLong(const char *p, bool *outOfRange)
{
    if (*p == '-') {
        ++p;
        return _StringToNegative<long>(p, outOfRange);
    }
    return _StringToPositive<long>(p, outOfRange);
}

long
TfStringToLong(const std::string &txt, bool *outOfRange)
{
    return TfStringToLong(txt.c_str(), outOfRange);
}

unsigned long
TfStringToULong(const char *p, bool *outOfRange)
{
    return _StringToPositive<unsigned long>(p, outOfRange);
}

unsigned long
TfStringToULong(const std::string &txt, bool *outOfRange)
{
    return TfStringToULong(txt.c_str(), outOfRange);
}

int64_t
TfStringToInt64(const char *p, bool *outOfRange)
{
    if (*p == '-') {
        ++p;
        return _StringToNegative<int64_t>(p, outOfRange);
    }
    return _StringToPositive<int64_t>(p, outOfRange);
}

int64_t
TfStringToInt64(const std::string &txt, bool *outOfRange)
{
    return TfStringToInt64(txt.c_str(), outOfRange);
}

uint64_t
TfStringToUInt64(const char *p, bool *outOfRange)
{
    return _StringToPositive<uint64_t>(p, outOfRange);
}

uint64_t
TfStringToUInt64(const std::string &txt, bool *outOfRange)
{
    return TfStringToUInt64(txt.c_str(), outOfRange);
}

bool
TfStringContains(const string &s, const char *substring)
{
    return s.find(substring) != string::npos;
}
bool
TfStringContains(const string &s, const TfToken &substring)
{
    return TfStringContains(s, substring.GetText());
}

string
TfStringToLower(const string &source)
{
    string lower;
    size_t length = source.length();
    
    lower.reserve(length);
    for (size_t i = 0; i < length; i++) {
        lower += tolower(source[i]);
    }

    return lower;
}

string
TfStringToUpper(const string &source)
{
    string upper;
    size_t length = source.length();
    
    upper.reserve(length);
    for (size_t i = 0; i < length; i++) {
        upper += toupper(source[i]);
    }

    return upper;
}

string
TfStringCapitalize(const string& source)
{
    if (source.empty()) {
        return source;
    }

    string result(source);
    result[0] = toupper(result[0]);

    return result;
}

string
TfStringGetCommonPrefix(string a, string b)
{
    if (b.length() < a.length())
        b.swap(a);

    std::pair<string::iterator, string::iterator> it =
        std::mismatch(a.begin(), a.end(), b.begin());

    return string(a.begin(), it.first);
}

string
TfStringGetSuffix(const string& name, char delimiter)
{
    size_t i = name.rfind(delimiter);
    if (i == string::npos)
        return "";
    else
        return name.substr(i+1);
}

string
TfStringGetBeforeSuffix(const string& name, char delimiter)
{
    size_t i = name.rfind(delimiter);
    if (i == string::npos)
        return name;
    else
        return name.substr(0, i);
}

string
TfGetBaseName(const string& fileName)
{
    if (fileName.empty())
        return fileName;
#if defined(ARCH_OS_WINDOWS)
    const string::size_type i = fileName.find_last_of("\\/");
#else
    const string::size_type i = fileName.rfind("/");
#endif
    if (i == fileName.size() - 1)    // ends in directory delimiter
        return TfGetBaseName(fileName.substr(0, i));
#if defined(ARCH_OS_WINDOWS)
    const std::wstring wfileName{ ArchWindowsUtf8ToUtf16(fileName) };
    LPWSTR result = PathFindFileNameW(wfileName.c_str());

    // If PathFindFilename returns the same string back, that means it didn't
    // do anything.  That could mean that the patch has no basename, in which
    // case we want to return the empty string, or it could mean that the
    // fileName was already basename, in which case we want to return the
    // string back.
    if (result == wfileName.c_str()) {
        const bool hasDriveLetter = fileName.find(":") != string::npos;
        const bool hasPathSeparator  = i != string::npos;
        if (hasDriveLetter || hasPathSeparator) {
            return std::string();
        }
    }
    return ArchWindowsUtf16ToUtf8(result);

#else
    if (i == string::npos)                      // no / in name
        return fileName;
    else
        return fileName.substr(i+1);
#endif
}

string
TfGetPathName(const string& fileName)
{
#if defined(ARCH_OS_WINDOWS)
    size_t i = fileName.find_last_of("\\/:");
#else
    size_t i = fileName.rfind("/");
#endif
    if (i == string::npos)                          // no / in name
        return "";
    else
        return fileName.substr(0, i+1);
}

string
TfStringTrimRight(const string& s, const char* trimChars)
{
    return s.substr(0, s.find_last_not_of(trimChars) + 1);
}

string
TfStringTrimLeft(const string &s, const char* trimChars)
{
    string::size_type i = s.find_first_not_of(trimChars);
    return (i == string::npos) ? string() : s.substr(i);
}

string
TfStringTrim(const string &s, const char* trimChars)
{
    string::size_type b = s.find_first_not_of(trimChars);
    if (b == string::npos) {
        return string();
    }
    return s.substr(b, s.find_last_not_of(trimChars) - b + 1);
}

string
TfStringReplace(const string& source, const string& from, const string& to)
{
    if (from.empty() || from == to) {
        return source;
    }

    string result = source;
    string::size_type pos = 0;

    while ((pos = result.find(from, pos)) != string::npos) {
        result.replace(pos, from.size(), to);
        pos += to.size();
    }
    return result;
}

string
TfStringJoin(const vector<string>& strings, const char* separator)
{
    return TfStringJoin(strings.begin(), strings.end(), separator);
}

string
TfStringJoin(const set<string>& strings, const char* separator)
{
    return TfStringJoin(strings.begin(), strings.end(), separator);
}

static inline
void _TokenizeToSegments(string const &src, char const *delimiters,
                         vector<pair<char const *, char const *> > &segments)
{
    // Delimiter checking LUT.
    // NOTE: For some reason, calling memset here is faster than doing the
    // aggregate initialization.  Beats me.  Ask gcc.  (10/07)
    char _isDelim[256]; // = {0};
    memset(_isDelim, 0, sizeof(_isDelim));
    for (char const *p = delimiters; *p; ++p)
        _isDelim[static_cast<unsigned char>(*p)] = 1;

#define IS_DELIMITER(c) (_isDelim[static_cast<unsigned char>(c)])

    // First build a vector of segments.  A segment is a pair of pointers into
    // \a src's data, the first indicating the start of a token, the second
    // pointing one past the last character of a token (like a pair of
    // iterators).
    
    // A small amount of reservation seems to help.
    segments.reserve(8);
    char const *end = src.data() + src.size();
    for (char const *c = src.data(); c < end; ++c) {
        // skip delimiters
        if (IS_DELIMITER(*c))
            continue;
        // have a token until the next delimiter.
        // push back a new segment, but we only know the begin point yet.
        segments.push_back(make_pair(c, c));
        for (++c; c != end; ++c)
            if (IS_DELIMITER(*c))
                break;
        // complete the segment with the end point.
        segments.back().second = c;
    }

#undef IS_DELIMITER
}
    
vector<string>
TfStringSplit(string const &src, string const &separator)
{
    vector<string> split;

    if (src.empty())
        return split;

    // XXX python throws a ValueError in this case, we exit silently.
    if (separator.empty())
        return split;

    size_t from=0;
    size_t pos=0;

    while (true) {
        pos = src.find(separator, from);
        if (pos == string::npos)
            break;
        split.push_back(src.substr(from, pos-from));
        from = pos + separator.size();
    }

    // Also add the 'last' substring
    split.push_back(src.substr(from));

    return split;
}

vector<string>
TfStringTokenize(string const &src, const char* delimiters)
{
    vector<pair<char const *, char const *> > segments;
    _TokenizeToSegments(src, delimiters, segments);

    // Construct strings into the result vector from the segments of src.
    vector<string> ret(segments.size());
    for (size_t i = 0; i != segments.size(); ++i)
        ret[i].append(segments[i].first, segments[i].second);
    return ret;
}

set<string>
TfStringTokenizeToSet(string const &src, const char* delimiters)
{
    vector<pair<char const *, char const *> > segments;
    _TokenizeToSegments(src, delimiters, segments);

    // Construct strings from the segments and insert them into the result.
    set<string> ret;
    for (size_t i = 0; i != segments.size(); ++i)
        ret.insert(string(segments[i].first, segments[i].second));

    return ret;
}

static size_t
_FindFirstOfNotEscaped(const string &source, const char *toFind, size_t offset)
{
    size_t pos = source.find_first_of(toFind, offset);

    while(pos != 0 && pos != string::npos && source[pos - 1] == '\\') {
        pos = source.find_first_of(toFind, pos + 1);
    }

    return pos;
}

vector<string>
TfQuotedStringTokenize(const string &source, const char *delimiters, 
                       string *errors)
{    
    vector<string> resultVec;
    size_t j, quoteIndex, delimIndex;
    const char *quotes = "\"\'`";
    string token;

    if (strpbrk(delimiters, quotes) != NULL) {
        if (errors != NULL)
            *errors = "Cannot use quotes as delimiters.";
        
        return resultVec;
    }
    
    string quote;
    for (size_t i = 0; i < source.length();) {
        // Eat leading delimiters.
        i = source.find_first_not_of(delimiters, i);

        if (i == string::npos) {
            // Nothing left but delimiters.
            break;  
        }

        quote.erase();
        token.erase();

        while ((quoteIndex = _FindFirstOfNotEscaped(source, quotes, i)) <
               (delimIndex = source.find_first_of(delimiters, i))) {

            // Push the token from 'i' until the first quote.
            if (i < quoteIndex)  
                token += source.substr(i, quoteIndex - i);

            // Find matching quote. Again, we skip quotes that have been
            // escaped with a preceding backslash.
            j = quoteIndex;
            quote = source[j];
            j = _FindFirstOfNotEscaped(source, quote.c_str(), j + 1);
            
            // If we've reached the end of the string, then we are
            // missing an end-quote.
            if (j == string::npos) {
                if (errors != NULL) {
                    *errors = TfStringPrintf(
                        "String is missing an end-quote (\'%s\'): %s", 
                        quote.c_str(), source.c_str());
                }
                resultVec.clear();
                return resultVec;
            }

            // Push the token between the quotes.
            if (quoteIndex + 1 < j)
                token += source.substr(quoteIndex + 1, j - (quoteIndex + 1));
            
            // Advance past the quote.
            i = j + 1;
        }

        // Push token.
        if (delimIndex == string::npos) 
            token += source.substr(i);   
        else
            token += source.substr(i, delimIndex - i);
        
        // If there are quote characters in 'token', we strip away any
        // preceding backslash before adding it to our results.     
        for(size_t q = 0; q < strlen(quotes); ++ q)
            token = TfStringReplace(token, string("\\") + quotes[q], 
                                    string() + quotes[q]);
        
        resultVec.push_back(token);
        
        if (delimIndex == string::npos) 
            break;
        else {
            // Set up for next loop.
            i = delimIndex + 1;
        }
    }
    return resultVec;
}

vector<string> 
TfMatchedStringTokenize(const string& source, 
                        char openDelimiter, 
                        char closeDelimiter, 
                        char escapeCharacter,
                        string *errors)
{
    vector<string> resultVec;

    if ((escapeCharacter == openDelimiter) || 
        (escapeCharacter == closeDelimiter)) {
        if (errors != NULL) 
            *errors = "Escape character cannot be a delimiter.";
        return resultVec;
    }

    // If a close delimiter appears before an open delimiter, and it's not
    // preceded by the escape character, we have mismatched delimiters.
    size_t closeIndex = source.find(closeDelimiter);
    if ((closeIndex != string::npos) && 
        ((closeIndex == 0) || (source[closeIndex - 1] != escapeCharacter)) &&
        (closeIndex < source.find(openDelimiter))) {
        if (errors != NULL) {
            *errors = TfStringPrintf(
                    "String has unmatched close delimiter ('%c', '%c'): %s", 
                    openDelimiter, closeDelimiter, source.c_str());
        }
        return resultVec;
    }

    bool sameDelimiters = (openDelimiter == closeDelimiter);

    string specialChars;
    if (escapeCharacter != '\0')
        specialChars += escapeCharacter;
    
    specialChars += openDelimiter;
    if (!sameDelimiters)
        specialChars += closeDelimiter;

    size_t openIndex = 0, nextIndex = 0;
    size_t openCount, closeCount;
    size_t sourceSize = source.size();

    while((openIndex = source.find(openDelimiter, openIndex)) != string::npos) {
        openCount = 1;
        closeCount = 0;
        nextIndex = openIndex;
        
        string token;
        while(closeCount != openCount) {
            nextIndex = source.find_first_of(specialChars, nextIndex + 1);
            if(nextIndex == string::npos) {
                if (errors != NULL) {
                    *errors = TfStringPrintf(
                      "String has unmatched open delimiter ('%c', '%c'): %s", 
                      openDelimiter, closeDelimiter, source.c_str());
                }
                resultVec.clear();
                return resultVec;
            }

            if (source[nextIndex] == escapeCharacter) {
                // Get character immediately after the escape character.
                size_t index = nextIndex + 1;
                if (index < sourceSize - 1) {
                    // Add the substring to 'token'. We remove the escape
                    // character but add the character immediately after it.
                    token += source.substr(openIndex + 1, 
                                           nextIndex - openIndex - 1) + 
                        source[index];
                        
                    // Reset indices for the next iteration.
                    openIndex = index;
                    nextIndex = index;
                }
            }            
            else if (!sameDelimiters && (source[nextIndex] == openDelimiter))
                openCount ++;
            else
                closeCount ++;
        }

        if (nextIndex > openIndex + 1) 
            token += source.substr(openIndex + 1, nextIndex - openIndex - 1);
            
        resultVec.push_back(token);
        openIndex = nextIndex + 1;        
    }

    // If a close delimiter appears after our last token, we have mismatched 
    // delimiters.
    closeIndex = source.find(closeDelimiter, nextIndex + 1);
    if ((closeIndex != string::npos) &&
        (source[closeIndex - 1] != escapeCharacter)) {
        if (errors != NULL) {
            *errors = TfStringPrintf(
                    "String has unmatched close delimiter ('%c', '%c'): %s", 
                    openDelimiter, closeDelimiter, source.c_str());
        }
        resultVec.clear();
        return resultVec;
    }

    return resultVec;
}

// Return true if ch is one of the characters '0'..'9', otherwise false.
static inline bool
IsDigit(unsigned ch) {
    return (ch - (unsigned)'0') < 10u;
}

static inline bool
IsAlpha(unsigned ch) {
    return ((ch & ~0x20) - (unsigned)'A') < 26u;
}

// Like std::mismatch, return a pair of pointers to the first different chars
// starting from b1 and b2.  If there is no difference, return e1 and b2 +
// distance(b1, e1).
static std::pair<char const *, char const *>
Mismatch(const char *b1, const char *e1, const char *b2)
{
    // By far, the most frequent case is that the inputs immediately mismatch.
    if (*b1 != *b2) {
        return { b1, b2 };
    }
    
    // We examine 8 bytes at a time until we find a difference, then scan for
    // where that difference occurs.
    //
    // The code may look ridiculous at first glance -- memcpy!?  But it turns
    // out that modern compilers optimize this to single 64-bit memory to
    // register move instructions.  So this is a nice way to get a simd-esque
    // approach without resorting to compiler/cpu specific intrinsics or inline
    // asm.
    constexpr size_t WordSz = sizeof(uint64_t);
    size_t numWords = (e1 - b1) / WordSz;
    int tail = (e1 - b1) % WordSz;
    // Check word-size chunks at a time.
    for (; numWords--; b1 += WordSz, b2 += WordSz) {
        uint64_t a, b, x;
        memcpy(&a, b1, WordSz);
        memcpy(&b, b2, WordSz);
        x = a ^ b;
        if (x) {
            int idx = ArchCountTrailingZeros(x) / WordSz;
            b1 += idx, b2 += idx;
            return { b1, b2 };
        }
    }
    // There will be a tail remainder if the string length is not a multiple of
    // 8, just scan it.
    switch (tail) {
    case 7: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 6: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 5: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 4: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 3: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 2: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    case 1: if (*b1 != *b2) { break; } else { ++b1, ++b2; }
    };
    return { b1, b2 };
}

bool
TfDictionaryLessThan::_LessImpl(const string& lstr, const string& rstr) const
{
    // Note that this code is fairly performance sensitive for certain
    // use-cases, so be careful making changes here.
    auto lcur = lstr.c_str(), rcur = rstr.c_str();

    // General case.
    size_t lsize = lstr.size(), rsize = rstr.size();
    auto lend = lcur + lsize, rend = rcur + rsize;

    auto curEnd = lcur + std::min(lsize, rsize);
    
    // Find the next differing byte in the strings.
    std::tie(lcur, rcur) = Mismatch(lcur, curEnd, rcur);
    // If the strings are identical, we're done.
    if (lcur == curEnd && lsize == rsize) {
        return false;
    }

    char l, r;

    while (true) {
        if (lcur == curEnd) {
            // If we have hit the end we're done.
            break;
        }
        l = *lcur, r = *rcur;
        // If they are letters that differ disregarding case, we're done.
        // but only if they are ASCII (i.e., the high bit is not set)
        if (_IsASCIIValue(l) && _IsASCIIValue(r) && (((l & ~0x20) != (r & ~0x20)) & bool(l & r & ~0x3f))) {
            // Add 5 mod 32 makes '_' sort before all letters.
            return ((l + 5) & 31) < ((r + 5) & 31);
        }
        else if (IsDigit(l) | IsDigit(r)) {
            if (IsDigit(l) & IsDigit(r)) {
                // We backtrack to find the start of each digit string, then we
                // scan each digit string, ignoring leading zeros to put the two
                // strings into alignment with their most significant digits.
                // Then we scan further and ignore digits while the digits
                // match.  At the first difference, we record the initial digit
                // of each, then count the remaining digits of each string.  The
                // lesser value is the one with the fewest digits, or the one
                // with the smaller initital digit if they have the same number
                // of digits.

                // Backtrack to start of digit sequence.
                char const *lDigStart = lcur, *rDigStart = rcur;
                char const *lDigEnd = lcur, *rDigEnd = rcur;
                char const *lBegin = lstr.c_str(), *rBegin = rstr.c_str();

                while (lDigStart != lBegin && IsDigit(lDigStart[-1])) {
                    --lDigStart;
                }
                while (rDigStart != rBegin && IsDigit(rDigStart[-1])) {
                    --rDigStart;
                }
                while (IsDigit(*lDigEnd) && lDigEnd != lend) {
                    ++lDigEnd;
                }
                while (IsDigit(*rDigEnd) && rDigEnd != rend) {
                    ++rDigEnd;
                }

                // Scan forward to find first significant digit (skip 0s).
                while (lDigStart != lDigEnd && *lDigStart == '0') {
                    ++lDigStart;
                }
                while (rDigStart != rDigEnd && *rDigStart == '0') {
                    ++rDigStart;
                }

                // Scan both digit strings while they have matching digits.
                while (lDigStart != lDigEnd && rDigStart != rDigEnd &&
                       IsDigit(*lDigStart) && *lDigStart == *rDigStart) {
                    ++lDigStart, ++rDigStart;
                }

                // If either but not both are at the end, then we can return.
                if ((lDigStart == lDigEnd) ^ (rDigStart == rDigEnd)) {
                    return lDigStart == lDigEnd && rDigStart != rDigEnd;
                }
                if (lDigStart != lDigEnd && rDigStart != rDigEnd) {
                    // Now we can record the most significant digits.
                    int lMSD = *lDigStart, rMSD = *rDigStart;
                    // Now l is less if l has fewer digits, or if it has the
                    // same number of digits and a smaller leading digit.
                    size_t digitsL = std::distance(lDigStart, lDigEnd);
                    size_t digitsR = std::distance(rDigStart, rDigEnd);
                    return std::tie(digitsL, lMSD) < std::tie(digitsR, rMSD);
                }
                // Otherwise values are equal and we continue.  Reset curEnd to
                // account for leading-zero count differences.
                lcur = lDigEnd, rcur = rDigEnd;
                curEnd = lcur + std::min(std::distance(lcur, lend),
                                         std::distance(rcur, rend));
            }
            else if (IsDigit(l) | IsDigit(r)) {
                if (lcur == lstr.c_str()) {
                    // special case could occur where one is a digit, but the other
                    // might be a UTF-8 character, so we have to treat them
                    // as unsigned in the comparison
                    return static_cast<unsigned char>(l) < static_cast<unsigned char>(r);
                }
                // If one is a digit (but not both), then we have to check the
                // preceding character to determine the outcome.  If the
                // preceding character is a digit, then lhs is less if rhs has
                // the digit.  If not, then lhs is less if it has the digit.
                auto prevChar = lcur[-1];

                return IsDigit(prevChar) ? IsDigit(r) : IsDigit(l);
            }
        }
        else if (!IsAlpha(l) || !IsAlpha(r)) {
            // At least one isn't a letter.
            // this either means it's an ASCII symbol (digit was checked above)
            // or a utf-8 encoded unicode character
            // we want ASCII symbols to sort first, so we can't treat the
            // utf-8 characters as unsigned (and all ASCII values remain the same
            // after the cast)
            return static_cast<unsigned char>(l) < static_cast<unsigned char>(r);
        }
        else {
            // Both letters, differ by case, continue.
            ++lcur, ++rcur;
        }
        // Find the next differing byte in the strings.
        std::tie(lcur, rcur) = Mismatch(lcur, curEnd, rcur);
    }

    // We are at the end of one or both, with at most 
    // only case/leading zero differences.  If only one at end, shorter
    // is less.
    if ((lcur != lend) || (rcur != rend)) {
        return lcur == lend;
    }

    // End of both -- use the first mismatch to discriminate.  It must be that
    // there's no difference, or it is either both numerals (one of them zero)
    // or different-case letters.
    std::tie(lcur, rcur) = Mismatch(
        lstr.c_str(), lstr.c_str() + std::min(lstr.size(), rstr.size()),
        rstr.c_str());
    
    l = *lcur, r = *rcur;
    return (r == '0') | ((l != '0') & (static_cast<unsigned char>(l) < static_cast<unsigned char>(r)));

}

std::string
TfStringify(bool v)
{
    return (v ? "true" : "false");
}

std::string
TfStringify(std::string const& s)
{
    return s;
}

static
const
pxr_double_conversion::DoubleToStringConverter& 
Tf_GetDoubleToStringConverter()
{
    static const pxr_double_conversion::DoubleToStringConverter conv(
        pxr_double_conversion::DoubleToStringConverter::NO_FLAGS,
        "inf", 
        "nan",
        'e',
        /* decimal_in_shortest_low */ -6,
        /* decimal_in_shortest_high */ 15,
        /* max_leading_padding_zeroes_in_precision_mode */ 0,
        /* max_trailing_padding_zeroes_in_precision_mode */ 0);

    return conv;
}

void
Tf_ApplyDoubleToStringConverter(float val, char* buffer, int bufferSize)
{
    const auto& conv = Tf_GetDoubleToStringConverter();
    pxr_double_conversion::StringBuilder builder(buffer, bufferSize);
    // This should only fail if we provide an insufficient buffer.
    TF_VERIFY(conv.ToShortestSingle(val, &builder),
              "double_conversion failed");
}

void
Tf_ApplyDoubleToStringConverter(double val, char* buffer, int bufferSize)
{
    const auto& conv = Tf_GetDoubleToStringConverter();
    pxr_double_conversion::StringBuilder builder(buffer, bufferSize);
    // This should only fail if we provide an insufficient buffer.
    TF_VERIFY(conv.ToShortest(val, &builder),
              "double_conversion failed");
}

std::string
TfStringify(float val)
{
    constexpr int bufferSize = 128;
    char buffer[bufferSize];
    Tf_ApplyDoubleToStringConverter(val, buffer, bufferSize);
    return std::string(buffer);
}

bool
TfDoubleToString(
    double val, char* buffer, int bufferSize, bool emitTrailingZero)
{
    if (bufferSize < 25) {
        return false;
    }
    using DSC = pxr_double_conversion::DoubleToStringConverter;
    int flags = DSC::NO_FLAGS;
    if (emitTrailingZero) {
        flags = DSC::EMIT_TRAILING_DECIMAL_POINT
            | DSC::EMIT_TRAILING_ZERO_AFTER_POINT;
    }
    const DSC conv(
        flags,
        "inf", 
        "nan",
        'e',
        /* decimal_in_shortest_low */ -6,
        /* decimal_in_shortest_high */ 15,
        /* max_leading_padding_zeroes_in_precision_mode */ 0,
        /* max_trailing_padding_zeroes_in_precision_mode */ 0);
    pxr_double_conversion::StringBuilder builder(buffer, bufferSize);
    // This should only fail if we provide an insufficient buffer.
    return conv.ToShortest(val, &builder);
}

std::string
TfStringify(double val)
{
    constexpr int bufferSize = 128;
    char buffer[bufferSize];
    Tf_ApplyDoubleToStringConverter(val, buffer, bufferSize);
    return std::string(buffer);
}

std::ostream& 
operator<<(std::ostream& o, TfStreamFloat t)
{
    constexpr int bufferSize = 128;
    char buffer[bufferSize];
    Tf_ApplyDoubleToStringConverter(t.value, buffer, bufferSize);
    return o << buffer;
}

std::ostream& 
operator<<(std::ostream& o, TfStreamDouble t)
{
    constexpr int bufferSize = 128;
    char buffer[bufferSize];
    Tf_ApplyDoubleToStringConverter(t.value, buffer, bufferSize);
    return o << buffer;
}

template <>
bool
TfUnstringify(const std::string &instring, bool*)
{
    return (strcmp(instring.c_str(), "true") == 0) ||
           (strcmp(instring.c_str(), "1") == 0) ||
           (strcmp(instring.c_str(), "yes") == 0) ||
           (strcmp(instring.c_str(), "on") == 0);
}

template <>
std::string
TfUnstringify(std::string const& s, bool*)
{
    return s;
}

string
TfStringGlobToRegex(const string& s)
{
    // Replace '.' by '\.', then '*' by '.*', and '?' by '.'
    // TODO: could handle {,,} and do (||), although these are not part of the
    // glob syntax.
    string ret(s);
    ret = TfStringReplace( ret, ".", "\\." );
    ret = TfStringReplace( ret, "*", ".*"  );
    ret = TfStringReplace( ret, "?", "."   );
    return ret;
}

/*
** Process escape sequences in ANSI C string constants. Ignores illegal
** escape sequence. Adapted from Duff code.
*/
static bool
_IsOctalDigit(const char c)
{
    return (('0' <= c) && (c <= '7'));
}

static unsigned char
_OctalToDecimal(const char c)
{
    return (c - '0');
}

static unsigned char
_HexToDecimal(const char c)
{
         if (('a' <= c) && (c <= 'f')) return ((c - 'a') + 10);
    else if (('A' <= c) && (c <= 'F')) return ((c - 'A') + 10);

    return (c - '0');
}

void
TfEscapeStringReplaceChar(const char** c, char** out)
{
    switch (*++(*c))
    {
        default:  *(*out)++ = **c; break;
        case '\\': *(*out)++ = '\\'; break; // backslash
        case 'a': *(*out)++ = '\a'; break; // bel
        case 'b': *(*out)++ = '\b'; break; // bs
        case 'f': *(*out)++ = '\f'; break; // np
        case 'n': *(*out)++ = '\n'; break; // nl
        case 'r': *(*out)++ = '\r'; break; // cr
        case 't': *(*out)++ = '\t'; break; // ht
        case 'v': *(*out)++ = '\v'; break; // vt
        case 'x':
        {
            // Allow only up to 2 hex digits.
            unsigned char n = 0;
            for (int nd = 0; isxdigit(*++(*c)) && nd != 2; ++nd) {
                n = (n * 16) + _HexToDecimal(**c);
            }
            --(*c);
            *(*out)++ = n;
            break;
        }
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        {
            // Allow only up to 3 octal digits.
            --(*c);
            unsigned char n = 0;
            for (int nd = 0; _IsOctalDigit(*++(*c)) && nd != 3; ++nd) {
                n = (n * 8) + _OctalToDecimal(**c);
            }
            --(*c);
            *(*out)++ = n;
            break;
        }
    }
}

std::string
TfEscapeString(const std::string &in)
{
    // We use type char and a deleter for char[] instead of just using
    // type char[] due to a (now fixed) bug in libc++ in LLVM.  See
    // https://llvm.org/bugs/show_bug.cgi?id=18350.
    std::unique_ptr<char,
                    std::default_delete<char[]>> out(new char[in.size()+1]);
    char *outp = out.get();

    for (const char *c = in.c_str(); *c; ++c)
    {
        if (*c != '\\') {
            *outp++ = *c;
            continue;
        }
        TfEscapeStringReplaceChar(&c,&outp);

    }
    *outp++ = '\0';
    return std::string(out.get(), outp - out.get() - 1);
}

string 
TfStringCatPaths( const string &prefix, const string &suffix )
{
    return TfNormPath(prefix + "/" + suffix);
}

std::string
TfMakeValidIdentifier(const std::string &in)
{
    std::string result;

    if (in.empty()) {
        result.push_back('_');
        return result;
    }

    result.reserve(in.size());
    char const *p = in.c_str();
    if (!(('a' <= *p && *p <= 'z') || 
          ('A' <= *p && *p <= 'Z') || 
          *p == '_')) {
        result.push_back('_');
    } else {
        result.push_back(*p);
    }

    for (++p; *p; ++p) {
        if (!(('a' <= *p && *p <= 'z') ||    
              ('A' <= *p && *p <= 'Z') ||  
              ('0' <= *p && *p <= '9') ||  
              *p == '_')) {
            result.push_back('_');
        } else {
            result.push_back(*p);
        }
    }
    return result;
}

std::string
TfGetXmlEscapedString(const std::string &in)
{
    if (in.find_first_of("&<>\"'") == std::string::npos)
        return in;

    std::string result;

    result = TfStringReplace(in,     "&",  "&amp;");
    result = TfStringReplace(result, "<",  "&lt;");
    result = TfStringReplace(result, ">",  "&gt;");
    result = TfStringReplace(result, "\"", "&quot;");
    result = TfStringReplace(result, "'",  "&apos;");

    return result;
}

PXR_NAMESPACE_CLOSE_SCOPE
