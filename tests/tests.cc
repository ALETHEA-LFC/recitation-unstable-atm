#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include "atm.hpp"
#include "catch.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
//                             Helper Definitions //
/////////////////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) {
    return false;  // file problem
  }

  std::string f1_read;
  std::string f2_read;
  while (f1.good() || f2.good()) {
    f1 >> f1_read;
    f2 >> f2_read;
    if (f1_read != f2_read || (f1.good() && !f2.good()) ||
        (!f1.good() && f2.good()))
      return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Test Cases
/////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Example: Create a new account", "[ex-1]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);

  Account sam_account = accounts[{12345678, 1234}];
  REQUIRE(sam_account.owner_name == "Sam Sepiol");
  REQUIRE(sam_account.balance == 300.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);
  std::vector<std::string> empty;
  REQUIRE(transactions[{12345678, 1234}] == empty);
}

TEST_CASE("Example: Simple widthdraw", "[ex-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.WithdrawCash(12345678, 1234, 20);
  auto accounts = atm.GetAccounts();
  Account sam_account = accounts[{12345678, 1234}];

  REQUIRE(sam_account.balance == 280.30);
}

TEST_CASE("Example: Print Prompt Ledger", "[ex-3]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto& transactions = atm.GetTransactions();
  transactions[{12345678, 1234}].push_back(
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $32000.00, Updated Balance: $72099.90");
  atm.PrintLedger("./prompt.txt", 12345678, 1234);
  REQUIRE(CompareFiles("./ex-1.txt", "./prompt.txt"));
}

TEST_CASE("RegisterAccount: duplicate throws", "[register]") {
  Atm atm;
  atm.RegisterAccount(1111, 2222, "Alice", 500.0);
  REQUIRE_THROWS_AS(atm.RegisterAccount(1111, 2222, "Bob", 100.0),
                    std::invalid_argument);
}
TEST_CASE("WithdrawCash: invalid account throws", "[withdraw-1]") {
  Atm atm;
  REQUIRE_THROWS_AS(atm.WithdrawCash(9999, 0000, 100.0), std::invalid_argument);
}

TEST_CASE("WithdrawCash: negative amount throws", "[withdraw-2]") {
  Atm atm;
  atm.RegisterAccount(3333, 4444, "Bob", 1000.0);
  REQUIRE_THROWS_AS(atm.WithdrawCash(3333, 4444, -50.0), std::invalid_argument);
}

TEST_CASE("WithdrawCash: overdraw throws runtime_error", "[withdraw-3]") {
  Atm atm;
  atm.RegisterAccount(3333, 4444, "Bob", 100.0);
  REQUIRE_THROWS_AS(atm.WithdrawCash(3333, 4444, 200.0), std::runtime_error);
}

TEST_CASE("WithdrawCash: transaction recorded", "[withdraw-4]") {
  Atm atm;
  atm.RegisterAccount(3333, 4444, "Bob", 1000.0);
  atm.WithdrawCash(3333, 4444, 200.0);
  auto& txns = atm.GetTransactions();
  REQUIRE(txns[{3333, 4444}].size() == 1);
}
TEST_CASE("DepositCash: negative amount throws", "[deposit-1]") {
  Atm atm;
  atm.RegisterAccount(5555, 6666, "Carol", 200.0);
  REQUIRE_THROWS_AS(atm.DepositCash(5555, 6666, -50.0), std::invalid_argument);
}

TEST_CASE("DepositCash: invalid account throws", "[deposit-2]") {
  Atm atm;
  REQUIRE_THROWS_AS(atm.DepositCash(9999, 0000, 100.0), std::invalid_argument);
}

TEST_CASE("DepositCash: balance updated", "[deposit-3]") {
  Atm atm;
  atm.RegisterAccount(5555, 6666, "Carol", 200.0);
  atm.DepositCash(5555, 6666, 300.0);
  REQUIRE(atm.CheckBalance(5555, 6666) == 500.0);
}

TEST_CASE("DepositCash: transaction recorded", "[deposit-4]") {
  Atm atm;
  atm.RegisterAccount(5555, 6666, "Carol", 200.0);
  atm.DepositCash(5555, 6666, 300.0);
  auto& txns = atm.GetTransactions();
  REQUIRE(txns[{5555, 6666}].size() == 1);
}
TEST_CASE("PrintLedger: invalid account throws", "[ledger-1]") {
  Atm atm;
  REQUIRE_THROWS_AS(atm.PrintLedger("test.txt", 9999, 0000),
                    std::invalid_argument);
}

TEST_CASE("PrintLedger: format check", "[ledger-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.WithdrawCash(12345678, 1234, 200.40);
  atm.DepositCash(12345678, 1234, 40000.00);
  atm.DepositCash(12345678, 1234, 32000.00);
  atm.PrintLedger("./ledger_test.txt", 12345678, 1234);

  std::ifstream file("./ledger_test.txt");
  REQUIRE(file.is_open());

  std::string line;

  std::getline(file, line);
  REQUIRE(line == "Name: Sam Sepiol");

  std::getline(file, line);
  REQUIRE(line == "Card Number: 12345678");

  std::getline(file, line);
  REQUIRE(line == "PIN: 1234");

  std::getline(file, line);
  REQUIRE(line == "----------------------------");
}