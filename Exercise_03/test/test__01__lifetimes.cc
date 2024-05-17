#include <catch2/catch_test_macros.hpp>

#include <iomanip>
#include <iostream>

using namespace std;

class Tracker
{
  public:
    Tracker ()
    { sm_N_default_ctors++; }

    Tracker (Tracker const &)
    { sm_N_copy_ctors++; }
    Tracker (Tracker &&) noexcept
    { sm_N_move_ctors++; }

    Tracker & operator= ([[maybe_unused]] Tracker const & orig)
    { sm_N_copy_assigns++; return *this; }
    Tracker & operator= ([[maybe_unused]] Tracker && orig) noexcept
    { sm_N_move_assigns++; return *this; }

    ~Tracker ()
    { sm_N_dtors++; }

  public:
    static void reset_counts ()
    {
      sm_N_default_ctors = 0;
      sm_N_copy_ctors    = 0;
      sm_N_move_ctors    = 0;
      sm_N_copy_assigns  = 0;
      sm_N_move_assigns  = 0;
      sm_N_dtors         = 0;
    }

  public:
    static int32_t sm_N_default_ctors;
    static int32_t sm_N_copy_ctors;
    static int32_t sm_N_move_ctors;
    static int32_t sm_N_copy_assigns;
    static int32_t sm_N_move_assigns;
    static int32_t sm_N_dtors;
};

int32_t Tracker::sm_N_default_ctors = 0;
int32_t Tracker::sm_N_copy_ctors    = 0;
int32_t Tracker::sm_N_move_ctors    = 0;
int32_t Tracker::sm_N_copy_assigns  = 0;
int32_t Tracker::sm_N_move_assigns  = 0;
int32_t Tracker::sm_N_dtors         = 0;

TEST_CASE("lifecycle events")
{
  Tracker::reset_counts();

  SECTION("scenario 1")
  {
    Tracker t_1;

    CHECK(Tracker::sm_N_default_ctors == 1); // The default constructor was called
    CHECK(Tracker::sm_N_copy_ctors    == 0); // No other ctors were called here
    CHECK(Tracker::sm_N_move_ctors    == 0); // 
    CHECK(Tracker::sm_N_copy_assigns  == 0); // No copy or move assignments were made
    CHECK(Tracker::sm_N_move_assigns  == 0); // 
    CHECK(Tracker::sm_N_dtors         == 0); // The object is still in scope so no dtor has been called yet at this point
  }

  SECTION("scenario 2")
  {
    {
      Tracker t_1;
      Tracker t_2 = t_1;
    }

    CHECK(Tracker::sm_N_default_ctors == 1); // Only 1 default ctor was called (line 73)
    CHECK(Tracker::sm_N_copy_ctors    == 1); // 1 copy ctor was called (line 74)
    CHECK(Tracker::sm_N_move_ctors    == 0); // No move ctors
    CHECK(Tracker::sm_N_copy_assigns  == 0); // No copy or move assigns were called
    CHECK(Tracker::sm_N_move_assigns  == 0); // 
    CHECK(Tracker::sm_N_dtors         == 2); // Both objects went out of scope and got destroyed at line 74 (at the closing brace)
  }

  SECTION("scenario 3")
  {
    {
      Tracker t_1 = Tracker();
      Tracker t_2(t_1);
    }

    CHECK(Tracker::sm_N_default_ctors == 1); // Line 88 is using a default ctor (Tracker()) -- ctor elision allows the object to be created directly into the ctor
    CHECK(Tracker::sm_N_copy_ctors    == 1); // Line 89 is constructing t_2 from t_1
    CHECK(Tracker::sm_N_move_ctors    == 0); // No move ctors, or copy/move assigns
    CHECK(Tracker::sm_N_copy_assigns  == 0); //
    CHECK(Tracker::sm_N_move_assigns  == 0); // 
    CHECK(Tracker::sm_N_dtors         == 2); // Both objects went out of scope at the closing brace on line 90 and were destroyed
  }

  SECTION("scenario 4")
  {
    {
      Tracker t_1;
      Tracker t_2 = std::move(t_1);
      t_1 = Tracker();
    }

    CHECK(Tracker::sm_N_default_ctors == 2); // Default ctor called on line 103 and 105
    CHECK(Tracker::sm_N_copy_ctors    == 0); // None called
    CHECK(Tracker::sm_N_move_ctors    == 1); // 1 called on line 104 -- we're constructing the Tracker object t_2 using move(t_1)
    CHECK(Tracker::sm_N_copy_assigns  == 0); // None called
    CHECK(Tracker::sm_N_move_assigns  == 1); // compiler does a move assignment on line 105 instead of a copy assign since it recognizes Tracker() is only used once then destroyed
    CHECK(Tracker::sm_N_dtors         == 3); // 3 objects were created within this scope so 3 were destroyed at the closing brace
  }

  SECTION("scenario 5")
  {
    {
      Tracker t_1 = Tracker();
      Tracker t_2 = std::move(t_1);
      Tracker t_3 = t_1;
    }

    CHECK(Tracker::sm_N_default_ctors == 1); // constructed t_1 on line 119 
    CHECK(Tracker::sm_N_copy_ctors    == 1); // constructing t_3 by copying t_1 on line 121
    CHECK(Tracker::sm_N_move_ctors    == 1); // moved t_1 to construct t_2
    CHECK(Tracker::sm_N_copy_assigns  == 0); // No assignments made
    CHECK(Tracker::sm_N_move_assigns  == 0); // None called
    CHECK(Tracker::sm_N_dtors         == 3); // t_1, t_2, t_3 were destroyed. Elision was used when Tracker() was used to construct t_1. From cppreference.com, "it is constructed directly into the storage of its final destination" so "no copy/move is performed".
  }

  SECTION("scenario 6")
  {
    {
      Tracker t_1 = Tracker();
      Tracker t_2 = t_1;
      t_2 = std::move(t_1);
    }

    CHECK(Tracker::sm_N_default_ctors == 1); // Construction of t_1 on line 135
    CHECK(Tracker::sm_N_copy_ctors    == 1); // Construction of t_2 on line 136
    CHECK(Tracker::sm_N_move_ctors    == 0); // None called
    CHECK(Tracker::sm_N_copy_assigns  == 0); // None called
    CHECK(Tracker::sm_N_move_assigns  == 1); // Assigning t_2 via moving t_1 on line 137
    CHECK(Tracker::sm_N_dtors         == 2); // t_1 has no guts but still exists, therefore t_1 and t_2 dtors are called at the closing brace
  }
}


