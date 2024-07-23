#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <doctest/trompeloeil.hpp>

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
  trompeloeil::sequence seq1;

  REQUIRE_CALL(mock::thing, callme(6)).RETURN(1).IN_SEQUENCE(seq1);
  REQUIRE_CALL(mock::thing, callme(ANY(int))).RETURN(3).IN_SEQUENCE(seq1);
  REQUIRE(callsit(1) == 3);
}
