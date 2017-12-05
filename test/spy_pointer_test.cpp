#include <libmodules/spy_pointer.hpp>
#include <gmock/gmock.h>

using namespace modules;

struct SpiedObject
  : public enable_spying
{
  virtual void foo() {}
};

struct MockSpiedObject
  : public SpiedObject
{
  MOCK_METHOD0(foo, void());
};

TEST(spying, can_dereference_spy)
{
  MockSpiedObject object;
  spy_pointer<MockSpiedObject> spy(&object);
  EXPECT_CALL(object, foo()).Times(1);
  spy->foo();
}
