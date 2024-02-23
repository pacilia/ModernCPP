#include <iostream>
#include <coroutine>
#include <optional>

struct PrimeGenerator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::optional<int> value;
        auto get_return_object() { return PrimeGenerator{ handle_type::from_promise(*this) }; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
        auto yield_value(int value) {
            this->value = value;
            return std::suspend_always{};
        }
    };

    bool move_next() { return coro ? (coro.resume(), !coro.done()) : false; }
    int current_value() { return *coro.promise().value; }

    PrimeGenerator(handle_type h) : coro(h) {}
    PrimeGenerator(PrimeGenerator&) = delete;
    PrimeGenerator(PrimeGenerator&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~PrimeGenerator() { if (coro) coro.destroy(); }

private:
    handle_type coro;
};

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

PrimeGenerator generate_primes(int limit) {
    co_yield 2;
    for (int i = 3; i <= limit; i += 2) {
        if (is_prime(i)) {
            co_yield i;
        }
    }
}

int main() {
    for (auto primes = generate_primes(1000); primes.move_next();) {
        std::cout << primes.current_value() << '\n';
    }
    return 0;
}