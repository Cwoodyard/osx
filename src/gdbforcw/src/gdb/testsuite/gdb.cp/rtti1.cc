/* Code to go along with tests in rtti.exp.
   
   Copyright 2003, 2004 Free Software Foundation, Inc.

   Contributed by David Carlton <carlton@bactrian.org> and by Kealia,
   Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "rtti.h"

namespace n1 {

  class C1;

  class Base1 {
  public:
    virtual ~Base1() { }
  };


  class C1: public Base1 {
  public:
  };

  class D1 : public C1{
  public:
    D1(C1 *, C1 *);
    
    C1* expr_1_;
    C1* expr_2_;
  };
  
  D1::D1(C1 *expr_1, C1 *expr_2)
    : expr_1_(expr_1), expr_2_(expr_2) { }

  C1 *create1() {
    return new D1(0, 0);
  }

} // n1

// NOTE: carlton/2004-01-23: This call exists only to convince GCC to
// keep around a reference to 'obj' in n2::func - GCC 3.4 had been
// optimizing it away.
void refer_to (n2::C2 *obj)
{
  // Do nothing.
}

namespace n2
{
  void func ()
  {
    C2 *obj = create2 ();

    refer_to (obj);			// func-constructs-done

    return;
  }
}

int main()
{
    using namespace n1;
    using namespace n2;

    C1 *e1 = create1();
    C2 *e2 = create2();

    n2::func();				// main-constructs-done

    return 0;
}
