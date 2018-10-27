#pragma once

#include <nonstd/optional.hpp>

#include "inc/essential.h"
#include "util/bytes.h"

class Storage
{
public:
  virtual ~Storage() {}

  // Save or create value to key.
  virtual void put(const Hash& key, const std::string& val) = 0;

  // Get value from key or return nullopt if not exist.
  virtual nonstd::optional<std::string> get(const Hash& key) const = 0;

  // Delete key from storage.
  virtual void del(const Hash& key) = 0;

  // Commit tx_transaction
  virtual void commit_block() = 0;

  virtual void switch_to_tx() = 0;
  virtual void switch_to_check() = 0;
  virtual void switch_to_query() = 0;

  virtual void save_protected_key(const std::string& key,
                                  const std::string& val) = 0;

  virtual nonstd::optional<std::string>
  get_protected_key(const std::string& key) = 0;
};