#pragma once

#include "util/buffer.h"

class NetApplication
{
public:
  //! TODO: comment
  virtual ~NetApplication() {}

  //! TODO: comment
  virtual bool process(Buffer& read_buffer, Buffer& write_buffer) = 0;
};
