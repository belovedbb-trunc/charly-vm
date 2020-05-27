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

import "math"

export = ->(Base) {
  return class Number extends Base {

    /*
     * Calls the callback *self* times, passing the current iteration count
     * as the first argument
     * */
    times(cb) {
      let i = 0

      while i < self {
        cb(i)
        i += 1
      }

      null
    }

    /*
     * The same as times, but shorter
     * */
    n(cb) = @times(cb)

    /*
     * Calls the callback with each number from *self* down to *num*
     * Both *self* and num are inclusive
     * */
    downto(num, callback) {
      let i = self

      while i >= num {
        callback(i)
        i -= 1
      }

      self
    }

    /*
     * Calls the callback with each number from *self* up to *num*
     * Both *self* and num are inclusive
     * */
    upto(num, callback) {
      let i = self

      while i <= num {
        callback(i)
        i += 1
      }

      self
    }

    /*
     * Returns the absolute value of this number
     * */
    abs {
      if self < 0 return -self
      self
    }

    /*
     * Returns the next highest integer
     * */
    ceil {
      math.ceil(self)
    }

    /*
     * Returns the next lowest integer
     * */
    floor {
      math.floor(self)
    }

    /*
     * Rounds this number to the nearest integer
     * */
    round {
      const r = self % 1
      if r < 0.5 return math.floor(self)
      math.ceil(self)
    }

    /*
     * Returns the sign of this number
     * Returns 0 if self is 0
     * */
    sign {
      if self < 0 return -1
      if self > 0 return 1
      0
    }

    /*
     * Returns the bigger number
     * */
    max(o) {
      if o > self return o
      self
    }

    /*
     * Returns the smaller number
     * */
    min(o) {
      if o < self return o
      self
    }

    /*
     * Checks if self is close to *expected* with a given *delta*
     * */
    close_to(expected, delta) {
      (self - expected).abs() <= delta
    }

    /*
     * Return the number itself
     * */
    to_n {
      self
    }

    // Converters for different time scales
    nanosecond   = new Charly.time.Duration(self / 1000000)
    nanoseconds  = new Charly.time.Duration(self / 1000000)
    microsecond  = new Charly.time.Duration(self / 1000)
    microseconds = new Charly.time.Duration(self / 1000)
    millisecond  = new Charly.time.Duration(self)
    milliseconds = new Charly.time.Duration(self)
    second       = new Charly.time.Duration(self * 1000)
    seconds      = new Charly.time.Duration(self * 1000)
    minute       = new Charly.time.Duration(self * 1000 * 60)
    minutes      = new Charly.time.Duration(self * 1000 * 60)
    hour         = new Charly.time.Duration(self * 1000 * 60 * 60)
    hours        = new Charly.time.Duration(self * 1000 * 60 * 60)
    day          = new Charly.time.Duration(self * 1000 * 60 * 60 * 24)
    days         = new Charly.time.Duration(self * 1000 * 60 * 60 * 24)
    week         = new Charly.time.Duration(self * 1000 * 60 * 60 * 24 * 7)
    weeks        = new Charly.time.Duration(self * 1000 * 60 * 60 * 24 * 7)
    year         = new Charly.time.Duration(self * 1000 * 60 * 60 * 24 * 365)
    years        = new Charly.time.Duration(self * 1000 * 60 * 60 * 24 * 365)
  }
}
