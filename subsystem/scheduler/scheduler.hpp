/**
 * @brief this subsystem implements a bare-metal task escheduler
 * 
 * Author: Omid Kandelusy
 */
// ===================================================================================
#ifndef SCHEDULER_HEADER_GUARD
#define SCHEDULER_HEADER_GUARD
// ===================================================================================
// including the required header files

/** standard c header files */
#include <cstdint>
#include <cstddef>

/** driver header files */
#include "wait.h"

// ===================================================================================
// type, template, and macro defintions

enum class Sched_return_codes : uint8_t {
    success,
    nullpointer_encountered,
    all_slots_assigned,
    slot_index_too_large,
    slot_must_be_taken
};

enum class Slot_status : uint8_t {
    initialized,
    assigned,
    deleted    
};

typedef void (*slot_handler_cb_t)(void);


typedef struct{
    Slot_status status;
    slot_handler_cb_t cb;
} sched_slot_t;

template<int N>
class Escheduler{
    static_assert(N > 0, "class input N must be greater than zero");
    private:
    sched_slot_t sched_obj[N];
    volatile uint8_t scheduler_flag;

    void looper(){
        while (scheduler_flag == 1){

            for(int i=0; i<N; i++){
                if (sched_obj[i].status != Slot_status::assigned){
                    continue;
                } else {
                    sched_obj[i].cb();
                }

            }

        }
    }

    public:
    Escheduler() : scheduler_flag(0) {}

    Sched_return_codes init(){
        for (int i =0; i < N; i++){
            sched_obj[i].status = Slot_status::initialized;
            sched_obj[i].cb = NULL;
        }

        return Sched_return_codes::success;
    }

    Sched_return_codes allocate(slot_handler_cb_t cb, uint8_t *slot_handle){
        if (!cb || !slot_handle) return Sched_return_codes::nullpointer_encountered;
        for (int i=0; i < N; i++){
            if (sched_obj[i].status != Slot_status::assigned){
                sched_obj[i].status = Slot_status::assigned;
                sched_obj[i].cb = cb;
                *slot_handle = i;
                return Sched_return_codes::success;
            }
        }

        return Sched_return_codes::all_slots_assigned;
    }

    Sched_return_codes remove(uint8_t slot_handle){
        if (slot_handle >= N) return Sched_return_codes::slot_index_too_large;
        if (sched_obj[slot_handle].status != Slot_status::assigned){
            return Sched_return_codes::slot_must_be_taken;
        }
        sched_obj[slot_handle].status = Slot_status::deleted;
        sched_obj[slot_handle].cb = NULL;

        return Sched_return_codes::success;
    }

    void start(){
        scheduler_flag = 1;
        looper();
    }

    void stop(){
        scheduler_flag = 0;
    }

};



#endif