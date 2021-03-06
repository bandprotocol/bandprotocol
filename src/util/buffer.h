// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the LICENSE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <deque>
#include <type_traits>

#include "inc/essential.h"

/// Converts the given span of raw data into its hex representation
std::string bytes_to_hex(gsl::span<const byte> data);

class Buffer
{
public:
  Buffer();
  Buffer(gsl::span<const byte> data);

  template <typename T>
  static T deserialize(const std::string& raw_data)
  {
    Buffer buf(gsl::as_bytes(gsl::make_span(raw_data)));
    return buf.read<T>();
  }

  /// Serialize the given data into raw string. Use operator<< internally.
  template <typename T>
  static std::string serialize(const T& data)
  {
    Buffer buf;
    buf << data;
    return std::string((const char*)&buf.buf.front(), buf.buf.size());
  }

  /// Read this buffer as the given type.
  template <typename T>
  T read();

  gsl::span<byte> as_span()
  {
    return gsl::make_span(buf);
  }

  /// Expose this data as a read-only span. Note that if this is destroyed,
  /// the span will become invalid.
  gsl::span<const byte> as_const_span() const
  {
    return gsl::make_span(buf);
  }

  Buffer& operator<<(std::byte val);
  Buffer& operator>>(std::byte& val);
  Buffer& operator<<(const Buffer& data);
  Buffer& operator>>(Buffer& data);

  template <typename T>
  friend Buffer& operator<<(Buffer& buf, gsl::span<T> data)
  {
    auto data_bytes = gsl::as_bytes(data);
    buf.buf.insert(buf.buf.end(), data_bytes.begin(), data_bytes.end());
    return buf;
  }

  template <typename T>
  friend Buffer& operator>>(Buffer& buf, gsl::span<T> data)
  {
    if (buf.size_bytes() < data.size_bytes())
      throw Error("Buffer parse error");
    std::memcpy(data.data(), &buf.buf.front(), data.size_bytes());
    buf.consume(data.size_bytes());
    return buf;
  }

  /// Return whether this buffer is empty
  bool empty() const
  {
    return buf.empty();
  }

  /// Return the size of this buffer in bytes.
  gsl::span<const std::byte>::size_type size_bytes() const
  {
    return as_const_span().size_bytes();
  }

  /// Clear the content in this buffer.
  void clear()
  {
    buf.clear();
  }

  /// Consume the first length bytes of this buffer.
  void consume(size_t length)
  {
    // TODO: Make this be more efficient
    buf.erase(buf.begin(), buf.begin() + length);
  }

  std::string to_string() const
  {
    return bytes_to_hex(as_const_span());
  }

  std::string to_raw_string() const
  {
    return std::string((const char*)&buf.front(), buf.size());
  }

private:
  std::vector<std::byte> buf;
};

template <typename T>
T Buffer::read()
{
  T result;
  *this >> result;
  return result;
}

template <typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
Buffer& operator<<(Buffer& buf, T val)
{
  return buf << static_cast<std::underlying_type_t<T>>(val);
}

template <typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
Buffer& operator>>(Buffer& buf, T& val)
{
  std::underlying_type_t<T> _val;
  buf >> _val;
  val = T(_val);
  return buf;
}

template <typename T,
          typename std::enable_if_t<std::is_integral_v<typename T::_integral>,
                                    int> = 0>
Buffer& operator<<(Buffer& buf, T val)
{
  return buf << val._to_integral();
}

template <typename T,
          typename std::enable_if_t<std::is_integral_v<typename T::_integral>,
                                    int> = 0>
Buffer& operator>>(Buffer& buf, T& val)
{
  typename T::_integral _val;
  buf >> _val;
  val = T::_from_integral(_val);
  return buf;
}

template <typename T>
void varint_encode(Buffer& buf, T val)
{
  while (true) {
    std::byte towrite{uint8_t(val & 0x7F)};
    val >>= 7;
    if (val != 0) {
      buf << (towrite | std::byte{0x80});
    } else {
      buf << towrite;
      break;
    }
  }
}

template <typename T>
void varint_decode(Buffer& buf, T& val)
{
  val = 0;

  int idx = 0;
  for (const auto raw_byte : buf.as_span()) {
    int byte = std::to_integer<int>(raw_byte);
    val |= T(byte & 0x7F) << (7 * idx);
    if (!(byte & 0x80)) {
      buf.consume(1 + idx);
      return;
    }

    ++idx;
  }
  throw Error("Invalid varint decode");
}

template <typename T, typename... Args>
void add_buffer(Buffer& buf, T value, Args... others)
{
  buf << value;
  if constexpr (sizeof...(Args) > 0) {
    add_buffer(buf, others...);
  }
}

Buffer& operator<<(Buffer& buf, bool val);
Buffer& operator>>(Buffer& buf, bool& val);

Buffer& operator<<(Buffer& buf, uint8_t val);
Buffer& operator>>(Buffer& buf, uint8_t& val);

Buffer& operator<<(Buffer& buf, uint16_t val);
Buffer& operator>>(Buffer& buf, uint16_t& val);

Buffer& operator<<(Buffer& buf, uint32_t val);
Buffer& operator>>(Buffer& buf, uint32_t& val);

Buffer& operator<<(Buffer& buf, uint64_t val);
Buffer& operator>>(Buffer& buf, uint64_t& val);

Buffer& operator<<(Buffer& buf, const uint256_t& val);
Buffer& operator>>(Buffer& buf, uint256_t& val);

Buffer& operator<<(Buffer& buf, const std::string& val);
Buffer& operator>>(Buffer& buf, std::string& val);

template <typename P1, typename P2>
Buffer& operator<<(Buffer& buf, const std::pair<P1, P2>& val)
{
  return buf << val.first << val.second;
}
template <typename P1, typename P2>
Buffer& operator>>(Buffer& buf, std::pair<P1, P2>& val)
{
  return buf >> val.first >> val.second;
}

template <typename T>
Buffer& operator<<(Buffer& buf, const std::vector<T>& val)
{
  buf << (uint64_t)val.size();
  for (const auto& v : val) {
    buf << v;
  }
  return buf;
}

template <typename T>
Buffer& operator>>(Buffer& buf, std::vector<T>& val)
{
  val.resize(buf.read<uint64_t>());
  for (auto& v : val) {
    buf >> v;
  }
  return buf;
}
