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

import "unittest"

if ARGV.contains("help") {
  print("Charly Unit Test Framework")
  print("--hide-stacktraces       Do not show stacktraces in error outputs")
  print("--runtime-profiler       Execute the test suite many times in a loop")
  return 0
}

const result = unittest(->(describe, it, assert, context) {
  const testcases = [

    // Interpreter specs
    ["Arithmetic operations", "./interpreter/arithmetic.ch"],
    ["Bitwise operations",    "./interpreter/bitwise.ch"],
    ["Classes",               "./interpreter/classes.ch"],
    ["Comments",              "./interpreter/comments.ch"],
    ["Comparisons",           "./interpreter/comparisons.ch"],
    ["Exceptions",            "./interpreter/exceptions.ch"],
    ["External Files",        "./interpreter/external-files.ch"],
    ["Functions",             "./interpreter/functions.ch"],
    ["Guard",                 "./interpreter/guard.ch"],
    ["Identifiers",           "./interpreter/identifiers.ch"],
    ["Loops",                 "./interpreter/loops.ch"],
    ["Objects",               "./interpreter/objects.ch"],
    ["Primitives",            "./interpreter/primitives.ch"],
    ["Switch",                "./interpreter/switch.ch"],
    ["Sync",                  "./interpreter/sync.ch"],
    ["Ternary",               "./interpreter/ternary.ch"],
    ["Unless",                "./interpreter/unless.ch"],

    // Primitive data type library specs
    ["Array stdlib",          "./stdlib/primitives/array.ch"],
    ["String stdlib",         "./stdlib/primitives/string.ch"],
    ["Frame stdlib",          "./stdlib/primitives/frame.ch"],

    // Standard library specs
    ["ARGV & Environment", "./stdlib/argv-environ.ch"],
    ["Channels",           "./stdlib/channel.ch"],
    ["CircularBuffer",     "./stdlib/circular_buffer.ch"],
    ["StringBuffer",       "./stdlib/stringbuffer.ch"],
    ["Heaps",              "./stdlib/heaps.ch"],
    ["Path",               "./stdlib/path.ch"],
    ["Queue",              "./stdlib/queue.ch"]
  ]

  let N = 1
  if ARGV.contains("--runtime-profiler") {
    N = 50
  }

  // Loads and runs all the test cases sequentially
  N.times(->{
    testcases.each(->(test) {
      describe(test[1], ->{
        const module = import test[1]
        module(describe, it, assert, context)
      })
    })
  })
})

unittest.display_result(result)

exit()
