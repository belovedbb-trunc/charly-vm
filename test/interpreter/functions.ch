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

export = ->(describe, it, assert) {

  describe("lambda notation", ->{

    it("allows the parens to be omitted", ->{
      let foo = ->{}

      assert(typeof foo, "function")
      assert(typeof foo(), "null")
    })

    it("allows the curly braces to be ommited", ->{
      let foo = ->(a, b) a + b

      assert(typeof foo, "function")
      assert(foo(1, 2), 3)
    })

    it("allows both parens and curly braces to be ommited", ->{
      let bar = ->{}
      let foo = ->bar()
      foo()

      assert(typeof bar, "function")
      assert(typeof foo, "function")
      assert(foo(), null)
    })

  })

  it("calls functions", ->{
    func foo(x) = return Math.sqrt(x)

    func p(x) {
      foo(x)
    }

    assert(p(25), 5)
    assert(p(625), 25)
    assert(p(100), 10)
    assert(p(4), 2)
    assert(p(16), 4)
    assert(p(64), 8)
  })

  it("inserts the argument variable", ->{
    let received

    func foo() {
      received = arguments
    }

    foo(1, 2, 3, 4, 5)

    assert(received, [1, 2, 3, 4, 5])
  })

  it("inserts quick access identifiers", ->{
    let received

    func foo() {
      received = [
        $0, $1, $2, $3, $4
      ]
    }

    foo(1, 2, 3, 4, 5)

    assert(received, [1, 2, 3, 4, 5])
  })

  it("calls callbacks", ->{
    func g(n, c) {
      c("Got " + n)
    }

    g("f1", ->(m) assert(m, "Got f1"))
    g(500, ->(m) assert(m, "Got 500"))
    g([1, 2, 3], ->(m) assert(m, "Got [1, 2, 3]"))
  })

  describe("self value", ->{

    it("correctly sets self value inside anonymous functions", ->{
      class A {
        property a1

        c(a, cb) {
          cb(a, ->(a1) {
            self.a1 = a1
          })
        }
      }

      const o = new A(1)

      o.c(20, ->$1($0))
      assert(o.a1, 20)

      o.c(40, ->$1($0))
      assert(o.a1, 40)

      o.c(80, ->$1($0))
      assert(o.a1, 80)
    })

    it("sets self for regular functions", ->{
      class A {
        property v

        foo(a, cb) {
          cb(a, func {
            self.v = a
          })
        }
      }

      const a = new A()

      a.foo(20, ->$1($0))
      assert(a.v, 20)

      a.foo(40, ->$1($0))
      assert(a.v, 40)

      a.foo(50, ->$1($0))
      assert(a.v, 50)
    })

  })

  it("does consecutive call expressions", ->{
    func call_me() {
      func() {
        func() {
          25
        }
      }
    }

    assert(call_me()()(), 25)
  })

  describe("dynamic calls", ->{
    it("calls a function with a context and arguments", ->{
      const ctx = { v: 20 }
      func foo(a, b) = @v + a + b
      assert(foo.call(ctx, [1, 2]), 23)
    })

    it("asynchronously calls a function", ->{
      const ctx = { v: 20 }
      func foo(a, b) = @v + a + b

      const p = foo.call(ctx, [1, 2], true)
      const result = p.wait()
      assert(result, 23)
    })

    it("calls a cfunction", ->{
      const testfunc = @"charly.vm.testfunc"
      const arg = 0
      const result = testfunc(arg)

      assert(typeof result, "object")
      assert(Object.keys(result).similar(["a", "b", "c", "d"]))
      assert(result.a, 0)
      assert(result.b, 1)
      assert(result.c, 2)
      assert(result.d, 3)
    })

    it("asynchronously calls a cfunction", ->{
      const testfunc = @"charly.vm.testfunc"
      const p = testfunc.call(null, [0], true)

      const result = p.wait()
      assert(typeof result, "object")
      assert(Object.keys(result).similar(["a", "b", "c", "d"]))
      assert(result.a, 0)
      assert(result.b, 1)
      assert(result.c, 2)
      assert(result.d, 3)
    })
  })

  describe("default arguments", ->{

    it("applies default arguments", ->{
      func f(v = 25) = v
      assert(f(), 25)
      assert(f(5), 5)
    })

    it("applies default arguments in the correct order", ->{

      let i = 0
      func n {
        const v = i
        i += 1
        v
      }

      func f(a = n(), b = n(), c = n()) {
        return [a, b, c]
      }

      assert(f(),         [0, 1, 2])
      assert(f(1),        [1, 3, 4])
      assert(f(1, 2),     [1, 2, 5])
      assert(f(1, 2, 3),  [1, 2, 3])
      assert(f(),         [6, 7, 8])
    })

    it("references other arguments", ->{
      func f(a = 0, b = a + 1) = [a, b]
      assert(f(), [0, 1])
      assert(f(10), [10, 11])
      assert(f(1, 5), [1, 5])
    })

    it("initializes member properties", ->{
      class A {
        property name

        set_name(@name = "default name")
      }

      const a = new A()
      a.set_name()
      assert(a.name, "default name")

      a.set_name("leonard")
      assert(a.name, "leonard")
    })

    it("throws an exception when passing insufficent arguments", ->{
      func f(a, b, c = 25) = [a, b, c]

      let caught_exception = null
      try {
        f(1)
      } catch(e) {
        caught_exception = e
      }

      assert(typeof caught_exception, "object")
      assert(caught_exception.message, "Not enough arguments for function call")
    })

    it("arguments array does not contain default arguments", ->{
      func f(a, b, c = 1, d = 2) = arguments

      assert(f(1, 2),       [1, 2])
      assert(f(1, 2, 3),    [1, 2, 3])
      assert(f(1, 2, 3, 4), [1, 2, 3, 4])
    })

    it("correctly sets quick access identifiers", ->{
      func f(a = 1, b = 2, c = 3) = [$0, $1, $2]

      assert(f(),         [1, 2, 3])
      assert(f(5),        [5, 2, 3])
      assert(f(5, 5),     [5, 5, 3])
      assert(f(5, 5, 5),  [5, 5, 5])
    })

    it("sets default arguments in anonymous functions", ->{
      const f = ->(a = 1, b = 2) a + b

      assert(f(), 3)
      assert(f(5), 7)
      assert(f(5, 5), 10)
    })

  })

  it("allows single expression blocks", ->{
    func foo = 25
    assert(foo(), 25)

    func bar() = 50
    assert(bar(), 50)
  })

  it("sets host_class property on class member functions & constructor", ->{
    class A {
      constructor = null
      foo = null
    }

    assert(A.constructor.host_class, A)

    const a = new A()
    assert(a.foo.host_class, A)
  })

  it("assigns bound_self to anonymous functions", ->{
    const method = ->{ self }
    assert(method(), self)
    method.bind_self(25)
    assert(method(), 25)
    method.unbind_self()
    assert(method(), self)
  })

  it("assigns bound_self of class member functions", ->{
    class A {
      property value
      foo = @value
    }

    const a = new A("test")
    assert(a.foo(), "test")

    a.foo.bind_self({ value: "something else" })
    assert(a.foo(), "something else")

    a.foo.unbind_self()
    assert(a.foo(), "test")
  })

  it("copies functions", ->{
    func foo = self

    const a = foo.bind_self("hello world")
    const b = a.copy()

    assert(a(), "hello world")
    assert(b(), "hello world")

    a.bind_self("test")

    assert(a(), "test")
    assert(b(), "hello world")
  })
}
