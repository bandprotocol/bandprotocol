#pragma once

#include <nonstd/optional.hpp>
#include <unordered_map>

#include "store/storage.h"

class StorageMap : public Storage
{
public:
  void put(const Hash& key, const std::string& val) final;
  nonstd::optional<std::string> get(const Hash& key) const final;
  void del(const Hash& key) final;

  void start_block() final {}

  void end_block() final {}

  void save_protected_key(const std::string& key, const std::string& val) final;
  nonstd::optional<std::string> get_protected_key(const std::string& key) final;

public:
  std::unordered_map<Hash, std::string> data;
};
