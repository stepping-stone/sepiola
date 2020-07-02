/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef UNICODE_TEXT_STREAM_HH
#define UNICODE_TEXT_STREAM_HH

/**
 * The UnicodeTextStream class provides a text stream with a unicode codec
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class UnicodeTextStream : public QTextStream
{
public:
    UnicodeTextStream(QIODevice *device);
    virtual ~UnicodeTextStream();
};

inline UnicodeTextStream::UnicodeTextStream(QIODevice *device)
    : QTextStream(device)
{
    this->setCodec("UTF-8");
}

inline UnicodeTextStream::~UnicodeTextStream() {}

#endif
