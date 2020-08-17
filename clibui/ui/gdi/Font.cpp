#include "stdafx.h"
#include "Gdi.h"

Font::Font()
    : size(0)
    , bold(false)
    , italic(false)
    , underline(false)
    , strikeline(false)
    , antialias(true)
    , verticalAntialias(true)
{

}

int Font::Compare(const Font& value) const
{
    int result = 0;

    result = fontFamily.Compare(value.fontFamily);
    if (result != 0) return result;

    result = size - value.size;
    if (result != 0) return result;

    result = (int)bold - (int)value.bold;
    if (result != 0) return result;

    result = (int)italic - (int)value.italic;
    if (result != 0) return result;

    result = (int)underline - (int)value.underline;
    if (result != 0) return result;

    result = (int)strikeline - (int)value.strikeline;
    if (result != 0) return result;

    result = (int)antialias - (int)value.antialias;
    if (result != 0) return result;

    return 0;
}

bool Font::operator>=(const Font& value) const
{
    return Compare(value) >= 0;
}

bool Font::operator>(const Font& value) const
{
    return Compare(value) > 0;
}

bool Font::operator<=(const Font& value) const
{
    return Compare(value) <= 0;
}

bool Font::operator<(const Font& value) const
{
    return Compare(value) < 0;
}

bool Font::operator!=(const Font& value) const
{
    return Compare(value) != 0;
}

bool Font::operator==(const Font& value) const
{
    return Compare(value) == 0;
}