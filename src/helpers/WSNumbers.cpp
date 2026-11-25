#include "helpers/WSNumbers.hpp"

bool WSNumbers::IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

// TODO: Spacing uses spaces rather than tabs in this file for some reason, fix at some point..
bool WSNumbers::IsHexDigit(char c) {
    return (WSNumbers::IsDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}