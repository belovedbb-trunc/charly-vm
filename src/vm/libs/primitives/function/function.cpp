/*
 * This file is part of the Charly Virtual Machine (https://github.com/KCreate/charly-vm)
 *
 * MIT License
 *
 * Copyright (c) 2017 - 2020 Leonard Schütz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "function.h"
#include "vm.h"

namespace Charly {
namespace Internals {
namespace PrimitiveFunction {

VALUE is_cfunc(VM&, VALUE func) {
  return charly_is_cfunction(func) ? kTrue : kFalse;
}

VALUE call(VM& vm, VALUE func, VALUE ctx, VALUE args) {
  CHECK(callable, func);
  CHECK(array, args);

  vm.push_stack(ctx);
  vm.push_stack(func);

  Array* args_array = charly_as_array(args);
  args_array->access_vector_shared([&](Array::VectorType* vec) {
    for (VALUE a : *vec) {
      vm.push_stack(a);
    }

    vm.call(vec->size(), true, false);
  });

  return kNull;
}

VALUE bind_self(VM& vm, VALUE func, VALUE self) {
  CHECK(function, func);

  Function* function = charly_as_function(func);
  function->set_bound_self(self);

  return func;
}

VALUE unbind_self(VM& vm, VALUE func) {
  CHECK(function, func);

  Function* function = charly_as_function(func);
  function->clear_bound_self();

  return func;
}

}  // namespace PrimitiveFunction
}  // namespace Internals
}  // namespace Charly
