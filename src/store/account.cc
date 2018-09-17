#include "account.h"

Account::Account(const Address& addr)
    : Object(addr)
{
}

uint256_t Account::get_balance(const TokenKey& token_key)
{
  const uint256_t value = balances[token_key];

  DEBUG(log, "GET_BALANCE:");
  DEBUG(log, "  Account: {}", key.to_iban_string(IBANType::Account));
  DEBUG(log, "  Token: {}", token_key.to_iban_string(IBANType::Contract));
  DEBUG(log, "  Value: {}", value);

  return value;
}

void Account::set_balance(const TokenKey& token_key, const uint256_t& value)
{
  DEBUG(log, "SET_BALANCE:");
  DEBUG(log, "  Account: {}", key.to_iban_string(IBANType::Account));
  DEBUG(log, "  Token: {}", token_key.to_iban_string(IBANType::Contract));
  DEBUG(log, "  Value: {}", value);

  balances[token_key] = value;
}
