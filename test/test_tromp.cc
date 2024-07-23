#include "catch2/catch_test_macros.hpp"
#include "trompeloeil/mock.hpp"
#include <catch2/trompeloeil.hpp>

#include "callsit.h"
#include "thing.h"

namespace mock {
class Thing {
public:
  MAKE_MOCK1(callme, int(int));
} thing;

extern "C" {
int callme(int a) { return thing.callme(a); }
}
} // namespace mock

TEST_CASE("Mocktest") {
  trompeloeil::sequence seq1, seq2;

  REQUIRE_CALL(mock::thing, callme(5)).RETURN(1).IN_SEQUENCE(seq1);
  REQUIRE_CALL(mock::thing, callme(ANY(int))).RETURN(3).IN_SEQUENCE(seq1);
  REQUIRE(callsit(1) == 3);
}
