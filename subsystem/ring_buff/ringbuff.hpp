/**
 * @brief this subsystem enales to deploy ring buffers in different projects
 * 
 * Author: Omid Kandelusy
 */
// ===================================================================================
// including the required header files

/** standard C++ header files */
#include <cstdint>
#include <cstddef>
#include <array>



// ===================================================================================
// typedefs, macros and templates


/** subsystems error code enums */
enum class Ringbuff_error : uint8_t{
    success,
    null_pointer,
    ringbuff_full,
    ringbuff_empty
};

/** overwrite or reject mode for the ring buffer */
enum class Operation_mode : uint8_t {
    reject,
    overwrite,
};


template <typename T, std::size_t capacity, Operation_mode mode>
class Ring_buffer{
    static_assert(capacity > 0, "Ringbuffer capacity must be greater than zero");

    public:

    bool is_full () const {
        return counter == capacity;
    }

    bool is_empty() const {
        return counter == 0;
    }

    Ringbuff_error push(const T& item){
        if (is_full()){
            if constexpr (mode == Operation_mode::reject) return Ringbuff_error::ringbuff_full;

            // updating the head to always pop the oldest buffered item:
            // this is for the overwrite mode:
            head = (head + 1) % capacity;
            counter --;
        }

        buffer[tail] = item;
        tail = (tail + 1) % capacity;
        counter++;

        return Ringbuff_error::success;
    }

    Ringbuff_error pop(T *item){
        if (!item) return Ringbuff_error::null_pointer;
        if (is_empty()) return Ringbuff_error::ringbuff_empty;

        *item = buffer[head];
        buffer[head] = T{};        // reset to default, important if T is a complicated type like a class
        head = (head + 1) % capacity;
        counter--;

        return Ringbuff_error::success;
    }

    private:
    std::array<T, capacity> buffer;
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t counter = 0;
};
