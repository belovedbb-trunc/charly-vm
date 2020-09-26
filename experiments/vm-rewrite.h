// Represents the stack of a single fiber
struct Stack {
  VALUE* data;
  uint32_t offset;
  uint32_t capacity;
};

// Represents the state of a single fiber / or thread of execution
// Stores frames, catchtables, the stack and instruction pointer
//
// Every fiber gets an ID, which uniquely identifies it across the runtime
// This id can be queried at runtime and can be used by other fibers
// to resume execution of this fiber
typedef uint64_t FIBER_ID;
struct Fiber {
  FIBER_ID    id;           // id of the fiber
  Stack       stack;        // stack containing temporary values
  Frame*      frames;       // function frame stack
  CatchTable* catchstack;   // catchstack for exceptions
  uint8_t*    ip;           // points to the currently executing instruction
  bool should_suspend;      // suspend the fiber on the next instruction
  bool should_terminate;    // terminate the fiber on the next instruction
  bool is_executing;        // wether this fiber is executing right now
  bool is_done;             // wether this fiber has terminated
};

// The different states a worker can be in
enum WorkerStatus {
  EXEC_CHARLY,            // executing charly code
  EXEC_NATIVE,            // executing native code
  EXEC_NATIVE_PROTECTED,  // executing native code, protected section
  EXEC_GC,                // executing garbage collection (only for GC worker)
  IDLE                    // doing nothing, waiting for work
};

// Signals a thread could be waiting on
enum WorkerWaitCondition {
  WAIT_FOR_GC,
  WAIT_FOR_PROTECTED,
  RUNNING
};

// Base class of any parallel workers
class Coordinator;
struct WorkerThread {
  Coordinator*        host_coordinator;   // host coordinator
  WorkerStatus        status;             // what this thread is currently doing
  WorkerWaitCondition wait_condition;     // what this thread is waiting for
  std::thread*        thread;             // OS thread handle
};

// Receives fibers from the coordinator's "ready_fibers" queue and starts
// executing them.
struct FiberWorker : public WorkerThread {
};

// Receives items from the coordinator's task queue and executes
// them in parallel to charly code
struct NativeWorker : public WorkerThread {
};

// Handles garbage collection and memory allocation related stuff
struct GCWorker : public WorkerThread {
};

// Asynchronous network handling
// Uses epoll on linux, kqueue on macOS
struct NetworkWorker : public WorkerThread {
};

// Represents a fiber which can be scheduled to resume executing
// Stores the ID of the fiber and an argument passed to it
struct FiberContinuation {
  FIBER_ID id;
  VALUE argument;
};

// Represents a function to be scheduled for execution
// Holds four argument values
struct FunctionArgumentPair {
  Function* function;
  VALUE arguments[4];
};

// Represents a native function to be called
// Stores the arguments and the fiber to be resumed with the result
struct NativeFunctionInvocation {
  CFunction* function;
  std::vector<VALUE> arguments;
  FIBER_ID source_fiber;
};

// Coordinates the execution of multiple fibers
// Handles timers, tickers
// Handles interactions with the work
class Coordinator {
  std::array<FiberWorker, 1> fiber_workers;
  std::array<NativeWorker, 4> native_workers;
  GCWorker gc_worker;
  NetworkWorker network_worker;

  // Paused fibers which are ready to be executed
  std::mutex ready_fibers_mutex;
  std::deque<FiberContinuation> ready_fibers;

  // Stores all fibers
  std::mutex fibers_mutex;
  std::unordered_map<FIBER_ID, Fiber*> fibers;

  // Queue of functions to be executed
  // The coordinator will create a new fiber and enqueue it in the
  // ready_fibers queue
  std::mutex task_queue_mutex;
  std::deque<FunctionArgumentPair> task_queue;

  // Scheduled timers / tickers
  std::map<Timestamp, FunctionArgumentPair> scheduled_timers;
  std::map<Timestamp, std::tuple<FunctionArgumentPair, uint32_t>> scheduled_tickers;
};
