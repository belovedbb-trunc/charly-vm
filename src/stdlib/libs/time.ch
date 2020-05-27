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

const __system_clock_now = Charly.internals.get_method("Time::system_clock_now")
const __steady_clock_now = Charly.internals.get_method("Time::steady_clock_now")
const __highres_now = Charly.internals.get_method("Time::highres_now")
const __to_local = Charly.internals.get_method("Time::to_local")
const __to_utc = Charly.internals.get_method("Time::to_utc")
const __fmt = Charly.internals.get_method("Time::fmt")
const __fmtutc = Charly.internals.get_method("Time::fmtutc")
const __parse = Charly.internals.get_method("Time::parse")

// Helper classes
class Duration {
  property ms

  in_nanoseconds  = @ms * 1000000
  in_microseconds = @ms * 1000
  in_milliseconds = @ms
  in_seconds      = @ms / 1000
  in_minutes      = @ms / (1000 * 60)
  in_hours        = @ms / (1000 * 60 * 60)
  in_days         = @ms / (1000 * 60 * 60 * 24)
  in_weeks        = @ms / (1000 * 60 * 60 * 24 * 7)
  in_years        = @ms / (1000 * 60 * 60 * 24 * 365)

  add(o) {
    new Duration(@ms + o.ms)
  }

  sub(o) {
    new Duration(@ms - o.ms)
  }

  mul(o) {
    new Duration(@ms * o)
  }

  div(o) {
    if typeof o == "number" return new Duration(@ms / o)
    @ms / o.ms
  }

  mod(o) {
    new Duration(@ms % o.ms)
  }

  @"+"(o) = @add(o)
  @"-"(o) = @sub(o)
  @"*"(o) = @mul(o)
  @"/"(o) = @div(o)
  @"%"(o) = @mod(o)
  @"="(o) = @ms == o.ms
  @"<"(o) = @ms < o.ms
  @">"(o) = @ms > o.ms
  @"<="(o) = @ms <= o.ms
  @">="(o) = @ms >= o.ms

  to_s {
    let remainder = self
    let buf = ""

    while remainder >= 1.nanosecond() {
      if remainder >= 1.year() {
        let c = Charly.math.floor(remainder / 1.year())
        buf += "" + c + " year" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.year()
      } else if remainder >= 1.day() {
        let c = Charly.math.floor(remainder / 1.day())
        buf += "" + c + " day" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.day()
      } else if remainder >= 1.hour() {
        let c = Charly.math.floor(remainder / 1.hour())
        buf += "" + c + " hour" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.hour()
      } else if remainder >= 1.minute() {
        let c = Charly.math.floor(remainder / 1.minute())
        buf += "" + c + " minute" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.minute()
      } else if remainder >= 1.second() {
        let c = Charly.math.floor(remainder / 1.second())
        buf += "" + c + " second" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.second()
      } else if remainder >= 1.millisecond() {
        let c = Charly.math.floor(remainder / 1.millisecond())
        buf += "" + c + " millisecond" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.millisecond()
      } else if remainder >= 1.microsecond() {
        let c = Charly.math.floor(remainder / 1.microsecond())
        buf += "" + c + " microsecond" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.microsecond()
      } else if remainder >= 1.nanosecond() {
        let c = Charly.math.floor(remainder / 1.nanosecond())
        buf += "" + c + " nanosecond" + ((c == 1) ? "" : "s") + " "
        remainder = remainder % 1.nanosecond()
      }
    }
    buf
  }
};

class Timestamp {
  property ms

  constructor(o) {
    if (typeof o == "object") {
      @ms = o.ms
      return
    }
    @ms = o
  }

  add(o) {
    new Timestamp(@ms + o.ms)
  }

  sub(o) {
    if o.klass == Duration return new Timestamp(@ms - o.ms)
    new Duration(@ms - o.ms)
  }

  @"+"(o) = @add(o)
  @"-"(o) = @sub(o)
  @"="(o) = @ms == o.ms
  @"<"(o) = @ms < o.ms
  @">"(o) = @ms > o.ms
  @"<="(o) = @ms <= o.ms
  @">="(o) = @ms >= o.ms

  floor(d) {
    const rem = @ms % d.ms
    new Timestamp(@ms - rem)
  }

  ceil(d) {
    const rem = @ms % d.ms
    new Timestamp(@ms + (d.ms - rem))
  }

  to_s {
    @to_utc()
  }

  to_local {
    __to_local(@ms)
  }

  to_utc {
    __to_utc(@ms)
  }

  fmt(format) {
    __fmt(@ms, format)
  }

  fmtutc(format) {
    __fmtutc(@ms, format)
  }
}

class Time {
  constructor {
    throw "Cannot initialize an instance of the Time class"
  }

  static now {
    new Timestamp(__system_clock_now())
  }

  static now_steady {
    new Timestamp(__steady_clock_now())
  }

  static now_highres {
    new Timestamp(__highres_now() / 1000000)
  }

  static parse(string, fmt) {
    new Timestamp(__parse(string, fmt))
  }

  static measure(cb) {
    const begin = Time.now_highres()
    cb()
    Time.now_highres() - begin
  }

  static property Timestamp = Timestamp
  static property Duration = Duration
}

export = Time
