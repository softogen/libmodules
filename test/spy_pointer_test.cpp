#include <libmodules/spy_pointer.hpp>
#include <gmock/gmock.h>

using namespace modules;

struct SpiedObject
  : public enable_spying<SpiedObject>
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
  spy_pointer<SpiedObject> spy(&object);
  EXPECT_CALL(object, foo()).Times(1);
  spy->foo();
}

TEST(spying, can_detect_spied_object_destruction)
{
  spy_pointer<SpiedObject> spy;
  ASSERT_FALSE(spy);
  {
    SpiedObject object;
    spy.reset(&object);
    ASSERT_TRUE(spy);
  }
  ASSERT_FALSE(spy);
}

